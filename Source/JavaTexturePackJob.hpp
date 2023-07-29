#pragma once

namespace mcview {

class JavaTexturePackJob : public TexturePackJob {
public:
  JavaTexturePackJob(juce::File const &worldDirectory,
                     juce::File const &mcaFile,
                     Region region,
                     Dimension dim,
                     bool useCache,
                     Delegate *delegate)
      : TexturePackJob(mcaFile.getFileName(), region, delegate),
        fWorldDirectory(worldDirectory),
        fDimension(dim),
        fRegionFile(mcaFile),
        fUseCache(useCache) {
  }

  ThreadPoolJob::JobStatus runJob() override {
    auto result = std::make_shared<Result>(fWorldDirectory, fDimension, fRegion);
    defer {
      fDelegate->texturePackJobDidFinish(result);
    };
    try {
      int64_t modified = fRegionFile.getLastModificationTime().toMilliseconds();
      juce::File cache = CacheFile(fWorldDirectory, fDimension, fRegion);
      if (fUseCache && cache.existsAsFile()) {
        if (LoadCache(result->fPixels, modified, cache)) {
          return ThreadPoolJob::jobHasFinished;
        }
      }

      auto mca = PathFromFile(fRegionFile);
      if (!std::filesystem::exists(mca)) {
        return ThreadPoolJob::jobHasFinished;
      }
      auto region = mcfile::je::Region::MakeRegion(mca);
      if (!region) {
        return ThreadPoolJob::jobHasFinished;
      }
      result->fPixels.reset(RegionToTexture::LoadJava(*region, fDimension, *this));
      if (shouldExit()) {
        return ThreadPoolJob::jobHasFinished;
      }
      if (result->fPixels) {
        StoreCache(result->fPixels.get(), modified, cache);
      }
      return ThreadPoolJob::jobHasFinished;
    } catch (std::exception &e) {
      juce::Logger::writeToLog(e.what());
      result->fPixels.reset();
      return ThreadPoolJob::jobHasFinished;
    } catch (...) {
      juce::Logger::writeToLog("Unknown error");
      result->fPixels.reset();
      return ThreadPoolJob::jobHasFinished;
    }
  }

private:
  juce::File const fWorldDirectory;
  Dimension const fDimension;
  juce::File const fRegionFile;
  bool const fUseCache;

  JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(JavaTexturePackJob)
};

} // namespace mcview
