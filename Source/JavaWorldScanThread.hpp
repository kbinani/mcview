#pragma once

namespace mcview {

class JavaWorldScanThread : public juce::Thread {
public:
  struct Delegate {
    virtual ~Delegate() = default;
    virtual void javaWorldScanThreadDidFoundRegion(juce::File worldDirectory, Dimension dimension, Region region) = 0;
    virtual void javaWorldScanThreadDidFinish(juce::File worldDirectory, Dimension dimension) = 0;
  };

  JavaWorldScanThread(juce::File worldDirectory, Dimension dimension, Delegate *delegate)
      : Thread("javaworldscanthread"),
        fWorldDirectory(worldDirectory),
        fDimension(dimension),
        fDelegate(delegate) {
  }

  void run() override {
    juce::RangedDirectoryIterator it(DimensionDirectory(fWorldDirectory, fDimension), false, "*.mca");
    for (juce::DirectoryEntry entry : it) {
      juce::File f = entry.getFile();
      auto r = mcfile::je::Region::MakeRegion(PathFromFile(f));
      if (!r) {
        continue;
      }
      if (threadShouldExit()) {
        return;
      }

      auto region = MakeRegion(r->fX, r->fZ);
      fDelegate->javaWorldScanThreadDidFoundRegion(fWorldDirectory, fDimension, region);
    }
    fDelegate->javaWorldScanThreadDidFinish(fWorldDirectory, fDimension);
  }

private:
  juce::File fWorldDirectory;
  Dimension fDimension;
  Delegate *const fDelegate;
};

} // namespace mcview
