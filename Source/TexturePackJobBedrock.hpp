#pragma once

namespace mcview {

class TexturePackJobBedrock : public TexturePackJob {
public:
  TexturePackJobBedrock(leveldb::DB *db, juce::File worldDirectory, Region region, Dimension dim, bool useCache, Delegate *delegate) : TexturePackJob("", delegate), fDb(db), fWorldDirectory(worldDirectory), fRegion(region), fDimension(dim), fUseCache(useCache) {
  }

  ThreadPoolJob::JobStatus runJob() override {
    auto result = std::make_shared<Result>(fWorldDirectory, fDimension, fRegion);
    try {
      juce::File cache = CacheFile(fWorldDirectory, fDimension, fRegion);
      if (fUseCache && cache.existsAsFile()) {
        if (LoadCache(result->fPixels, std::nullopt, cache)) {
          fDelegate->texturePackJobDidFinish(result);
        }
      }

      defer {
        fDelegate->texturePackJobDidFinish(result);
      };

      result->fPixels.reset(RegionToTexture::LoadBedrock(*fDb, fRegion.first, fRegion.second, this, fDimension));
      if (shouldExit()) {
        return ThreadPoolJob::jobHasFinished;
      }
      StoreCache(result->fPixels.get(), 0, cache);
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
  Region const fRegion;
  bool const fUseCache;

  JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(TexturePackJobBedrock)
};

} // namespace mcview
