#pragma once

namespace mcview {

class BedrockTexturePackJob : public TexturePackJob {
public:
  BedrockTexturePackJob(leveldb::DB *db,
                        juce::File worldDirectory,
                        Region region,
                        Dimension dim,
                        std::optional<int64_t> lastPlayed,
                        bool useCache,
                        Delegate *delegate)
      : TexturePackJob("", region, delegate),
        fDb(db),
        fWorldDirectory(worldDirectory),
        fDimension(dim),
        fLastPlayed(lastPlayed),
        fUseCache(useCache) {
  }

  ThreadPoolJob::JobStatus runJob() override {
    auto result = std::make_shared<Result>(fWorldDirectory, fDimension, fRegion);
    defer {
      fDelegate->texturePackJobDidFinish(result);
    };
    try {
      juce::File cache = CacheFile(fWorldDirectory, fDimension, fRegion);
      if (fUseCache && cache.existsAsFile()) {
        if (LoadCache(result->fPixels, fLastPlayed, cache)) {
          return ThreadPoolJob::jobHasFinished;
        }
      }

      result->fPixels.reset(RegionToTexture::LoadBedrock(*fDb, fRegion.first, fRegion.second, fDimension, *this));
      if (shouldExit()) {
        return ThreadPoolJob::jobHasFinished;
      }
      int64_t timestamp = (int64_t)floor(juce::Time::getCurrentTime().currentTimeMillis() / 1000.0);
      if (fLastPlayed) {
        timestamp = *fLastPlayed;
      }
      StoreCache(result->fPixels.get(), timestamp, cache);
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
  leveldb::DB *const fDb;
  juce::File const fWorldDirectory;
  Dimension const fDimension;
  std::optional<int64_t> const fLastPlayed;
  bool const fUseCache;

  JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(BedrockTexturePackJob)
};

} // namespace mcview
