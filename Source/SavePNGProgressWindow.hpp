#pragma once

namespace mcview {

class SavePNGProgressWindow : public juce::ThreadWithProgressWindow {
public:
  class Delegate {
  public:
    virtual ~Delegate() = default;
    virtual void savePNGProgressWindowRender(int const width, int const height, LookAt const lookAt) = 0;
    virtual void savePNGProgressWindowDidFinishRendering() = 0;
  };

  SavePNGProgressWindow(Delegate *delegate,
                        juce::OpenGLContext &openGLContext,
                        juce::File file,
                        int minRx, int minRz, int maxRx, int maxRz)
      : juce::ThreadWithProgressWindow(TRANS("Writing image file"), true, false),
        fDelegate(delegate),
        fGLContext(openGLContext),
        fFile(file),
        fMinRx(minRx), fMinRz(minRz), fMaxRx(maxRx), fMaxRz(maxRz) {
  }

  void run() override {
    using namespace juce;

    if (fMinRx > fMaxRx || fMinRz > fMaxRz) {
      return;
    }

    int const minBlockX = fMinRx * 512;
    int const minBlockZ = fMinRz * 512;
    int const maxBlockX = fMaxRx * 512 - 1;
    int const maxBlockZ = fMaxRz * 512 - 1;

    int const width = maxBlockX - minBlockX + 1;
    int const height = maxBlockZ - minBlockZ + 1;

    auto image = std::make_unique<Image>(Image::PixelFormat::ARGB, width, height, true);
    Image::BitmapData data(*image, Image::BitmapData::readWrite);
    if (data.lineStride != 4 * width || data.pixelStride != 4) {
      return;
    }

    std::latch latch(1);

    fGLContext.executeOnGLThread(
        [this, &latch, &data, minBlockX, minBlockZ, width, height](OpenGLContext &ctx) {
          std::unique_ptr<OpenGLFrameBuffer> buffer(new OpenGLFrameBuffer());
          buffer->initialise(ctx, width, height);
          buffer->makeCurrentRenderingTarget();

          LookAt lookAt;
          lookAt.fX = minBlockX + width / 2.0f;
          lookAt.fZ = minBlockZ + height / 2.0f;
          lookAt.fBlocksPerPixel = 1;
          fDelegate->savePNGProgressWindowRender(width, height, lookAt);

          buffer->readPixels((PixelARGB *)data.data, juce::Rectangle<int>(0, 0, width, height), OpenGLFrameBuffer::RowOrder::fromBottomUp);
          buffer->releaseAsRenderingTarget();

          buffer->release();
          buffer.reset();

          latch.count_down();

          fDelegate->savePNGProgressWindowDidFinishRendering();
        },
        false);

    latch.wait();
    setProgress(0.1);

    FileOutputStream stream(fFile);
    stream.setPosition(0);
    stream.truncate();
    PNGWriter writer(width, height, stream);
    for (int y = 0; y < height; y++) {
      writer.writeRow((PixelARGB *)data.getLinePointer(height - y - 1));
      double p = (y + 1) / (double)height;
      setProgress(0.1 + p * 0.9);
    }
    setProgress(1);
  }

private:
  Delegate *const fDelegate;
  juce::OpenGLContext &fGLContext;
  juce::File fFile;
  int fMinRx, fMinRz, fMaxRx, fMaxRz;

  JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(SavePNGProgressWindow)
};

} // namespace mcview
