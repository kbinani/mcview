#pragma once

namespace mcview {

class JavaWorldScanThread : public WorldScanThread {
public:
  struct Delegate {
    virtual ~Delegate() = default;
    virtual void javaWorldScanThreadDidFoundRegion(juce::File worldDirectory, Dimension dimension, Region region) = 0;
    virtual void javaWorldScanThreadDidFinish(juce::File worldDirectory, Dimension dimension) = 0;
  };

  JavaWorldScanThread(juce::File worldDirectory, Dimension dimension, Delegate *delegate)
      : WorldScanThread("Java World Scan Thread"),
        fWorldDirectory(worldDirectory),
        fDimension(dimension),
        fDelegate(delegate) {
  }

  void run() override {
    juce::RangedDirectoryIterator it(DimensionDirectory(fWorldDirectory, fDimension), false, "*.mca");
    for (juce::DirectoryEntry entry : it) {
      if (threadShouldExit()) {
        return;
      }
      juce::File f = entry.getFile();
      auto r = mcfile::je::Region::MakeRegion(PathFromFile(f));
      if (!r) {
        continue;
      }

      auto region = MakeRegion(r->fX, r->fZ);
      auto delegate = fDelegate.load();
      if (delegate) {
        delegate->javaWorldScanThreadDidFoundRegion(fWorldDirectory, fDimension, region);
      } else {
        return;
      }
    }
    auto delegate = fDelegate.load();
    if (delegate) {
      delegate->javaWorldScanThreadDidFinish(fWorldDirectory, fDimension);
    }
  }

  void abandon() override {
    fDelegate.store(nullptr);
  }

private:
  juce::File fWorldDirectory;
  Dimension fDimension;
  std::atomic<Delegate *> fDelegate;
};

} // namespace mcview
