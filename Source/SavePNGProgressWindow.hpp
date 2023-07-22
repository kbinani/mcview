#pragma once

namespace mcview {

class SavePNGProgressWindow : public juce::ThreadWithProgressWindow {
public:
  class Delegate {
  public:
    virtual ~Delegate() = default;
    virtual void savePNGProgressWindowRender(int const width, int const height, LookAt const lookAt) = 0;
  };

  SavePNGProgressWindow(Delegate *delegate, juce::OpenGLContext &openGLContext, juce::File file, juce::Rectangle<int> regionBoundingBox) : juce::ThreadWithProgressWindow(TRANS("Writing image file"), true, false), fDelegate(delegate), fGLContext(openGLContext), fFile(file), fRegionBoundingBox(regionBoundingBox) {
  }

  void run() {
    using namespace juce;
    auto bounds = fRegionBoundingBox;
    if (bounds.getWidth() == 0 || bounds.getHeight() == 0) {
      return;
    }

    int const minBlockX = bounds.getX() * 512;
    int const minBlockZ = bounds.getY() * 512;
    int const maxBlockX = (bounds.getRight() + 1) * 512 - 1;
    int const maxBlockZ = (bounds.getBottom() + 1) * 512 - 1;

    int const width = maxBlockX - minBlockX + 1;
    int const height = maxBlockZ - minBlockZ + 1;

    int const kMaxMemoryUsage = 32 * 1024 * 1024;
    int const row = std::max(kMaxMemoryUsage / ((int)sizeof(PixelARGB) * width), 128);

    std::vector<PixelARGB> pixels(width * row);
    PixelARGB *pixelsPtr = pixels.data();
    int const numFrames = (int)ceil(height / float(row));

    FileOutputStream stream(fFile);
    stream.truncate();
    stream.setPosition(0);
    PNGWriter writer(width, height, stream);

    fGLContext.executeOnGLThread([this, minBlockX, maxBlockX, minBlockZ, maxBlockZ, width, height, pixelsPtr, row, numFrames, &writer](OpenGLContext &ctx) {
      int y = 0;
      for (int i = 0; i < numFrames; i++) {
        std::fill_n(pixelsPtr, width, PixelARGB());

        std::unique_ptr<OpenGLFrameBuffer> buffer(new OpenGLFrameBuffer());
        buffer->initialise(ctx, width, row);
        buffer->makeCurrentRenderingTarget();

        LookAt lookAt;
        lookAt.fX = minBlockX + width / 2.0f;
        lookAt.fZ = minBlockZ + i * row + row / 2.0f;
        lookAt.fBlocksPerPixel = 1;
        fDelegate->savePNGProgressWindowRender(width, row, lookAt);

        buffer->readPixels(pixelsPtr, juce::Rectangle<int>(0, 0, width, row));
        buffer->releaseAsRenderingTarget();

        buffer->release();
        buffer.reset();

        for (int j = row; --j >= 0;) {
          writer.writeRow(pixelsPtr + width * j);
          y++;
          setProgress(y / float(height));
          if (y >= height) {
            break;
          }
        }
      }
    },
                                 true);

    setProgress(1);
  }

private:
  Delegate *const fDelegate;
  juce::OpenGLContext &fGLContext;
  juce::File fFile;
  juce::Rectangle<int> fRegionBoundingBox;
};

} // namespace mcview
