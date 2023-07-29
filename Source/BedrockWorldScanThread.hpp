#pragma once

namespace mcview {

class BedrockWorldScanThread : public WorldScanThread {
public:
  struct Delegate {
    virtual ~Delegate() = default;
    virtual void bedrockWorldScanThreadDidFoundRegion(juce::File worldDirectory, Dimension dim, Region region) = 0;
    virtual void bedrockWorldScanThreadDidFinish(juce::File worldDirectory, Dimension dim) = 0;
  };

  BedrockWorldScanThread(std::shared_ptr<leveldb::DB> db, juce::File worldDirectory, Dimension dim, Delegate *delegate)
      : WorldScanThread("Bedrock World Scan Thread"),
        fDb(db),
        fWorldDirectory(worldDirectory),
        fDimension(dim),
        fDelegate(delegate) {}

  void run() override {
    std::set<Region> found;
    bool completed = mcfile::be::Chunk::ForAll(fDb.get(), DimensionFromDimension(fDimension), [this, &found](int cx, int cz) -> bool {
      if (threadShouldExit()) {
        return false;
      }
      int rx = mcfile::Coordinate::RegionFromChunk(cx);
      int rz = mcfile::Coordinate::RegionFromChunk(cz);
      auto region = MakeRegion(rx, rz);
      if (found.count(region) == 0) {
        found.insert(region);
        auto delegate = fDelegate.load();
        if (delegate) {
          delegate->bedrockWorldScanThreadDidFoundRegion(fWorldDirectory, fDimension, region);
        } else {
          return false;
        }
      }
      return true;
    });
    if (completed && !threadShouldExit()) {
      auto delegate = fDelegate.load();
      if (delegate) {
        delegate->bedrockWorldScanThreadDidFinish(fWorldDirectory, fDimension);
      }
    }
  }

  void abandon() override {
    fDelegate.store(nullptr);
  }

private:
  std::shared_ptr<leveldb::DB> fDb;
  juce::File const fWorldDirectory;
  Dimension const fDimension;
  std::atomic<Delegate *> fDelegate;
};

} // namespace mcview
