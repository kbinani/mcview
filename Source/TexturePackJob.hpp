#pragma once

namespace mcview {

class TexturePackJob : public juce::ThreadPoolJob {
public:
  class Result {
  public:
    Result(juce::File worldDirectory, Dimension dimension, Region region) : fWorldDirectory(worldDirectory), fDimension(dimension), fRegion(region) {}

    juce::File const fWorldDirectory;
    Dimension const fDimension;
    Region const fRegion;
    std::unique_ptr<juce::PixelARGB[]> fPixels;
  };

  class Delegate {
  public:
    virtual ~Delegate() = default;
    virtual void texturePackJobDidFinish(std::shared_ptr<Result> result) = 0;
  };

  TexturePackJob(juce::String name, Delegate *delegate) : juce::ThreadPoolJob(name), fDelegate(delegate) {}
  ~TexturePackJob() override = default;

protected:
  Delegate *const fDelegate;
};

class TexturePackJobJava : public TexturePackJob {
public:
  TexturePackJobJava(juce::File const &worldDirectory, juce::File const &mcaFile, Region region, Dimension dim, bool useCache, Delegate *delegate)
      : TexturePackJob(mcaFile.getFileName(), delegate), fWorldDirectory(worldDirectory), fDimension(dim), fRegion(region), fRegionFile(mcaFile), fUseCache(useCache) {
  }

  juce::ThreadPoolJob::JobStatus runJob() override {
    using namespace juce;

    auto result = std::make_shared<Result>(fWorldDirectory, fDimension, fRegion);
    defer {
      fDelegate->texturePackJobDidFinish(result);
    };
    try {
      int64 const modified = fRegionFile.getLastModificationTime().toMilliseconds();
      File cache = RegionToTexture::CacheFile(fRegionFile);
      if (fUseCache && cache.existsAsFile()) {
        FileInputStream stream(cache);
        GZIPDecompressorInputStream ungzip(stream);
        int expectedBytes = sizeof(PixelARGB) * 512 * 512;
        int64 cachedModificationTime = 0;
        if (ungzip.read(&cachedModificationTime, sizeof(cachedModificationTime)) != sizeof(cachedModificationTime)) {
          return ThreadPoolJob::jobHasFinished;
        }
        if (cachedModificationTime >= modified) {
          result->fPixels.reset(new PixelARGB[512 * 512]);
          if (ungzip.read(result->fPixels.get(), expectedBytes) != expectedBytes) {
            result->fPixels.reset();
          }
          return ThreadPoolJob::jobHasFinished;
        }
      }

      auto region = mcfile::je::Region::MakeRegion(PathFromFile(fRegionFile));
      if (!region) {
        return ThreadPoolJob::jobHasFinished;
      }
      result->fPixels.reset(RegionToTexture::LoadJava(*region, this, fDimension));
      if (shouldExit()) {
        return ThreadPoolJob::jobHasFinished;
      }
      FileOutputStream out(cache);
      out.truncate();
      out.setPosition(0);
      GZIPCompressorOutputStream gzip(out, 9);
      if (result->fPixels) {
        gzip.write(&modified, sizeof(modified));
        gzip.write(result->fPixels.get(), sizeof(PixelARGB) * 512 * 512);
      }
      return ThreadPoolJob::jobHasFinished;
    } catch (std::exception &e) {
      Logger::writeToLog(e.what());
      result->fPixels.reset();
      return ThreadPoolJob::jobHasFinished;
    } catch (...) {
      Logger::writeToLog("Unknown error");
      result->fPixels.reset();
      return ThreadPoolJob::jobHasFinished;
    }
  }

private:
  juce::File const fWorldDirectory;
  Dimension const fDimension;
  Region const fRegion;
  juce::File const fRegionFile;
  bool const fUseCache;

  JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(TexturePackJobJava)
};

} // namespace mcview
