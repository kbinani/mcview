#pragma once

namespace mcview {

class RegionUpdateChecker : public juce::Thread {
public:
  class Delegate {
  public:
    virtual ~Delegate() = default;
    virtual void regionUpdateCheckerDidDetectRegionFileUpdate(std::vector<juce::File> files, Dimension dimension) = 0;
  };

  explicit RegionUpdateChecker(Delegate *delegate)
      : juce::Thread("RegionUpdateChecker"), fDim(Dimension::Overworld), fDelegate(delegate) {
  }

  void run() override {
    std::map<std::string, int64_t> updated;

    while (!currentThreadShouldExit()) {
      try {
        juce::Thread::sleep(1000);
        checkUpdatedFiles(updated);
      } catch (...) {
      }
    }
  }

  void setDirectory(juce::File f, Dimension dim) {
    juce::ScopedLock lk(fSection);
    fDirectory = f;
    fDim = dim;
  }

private:
  void checkUpdatedFiles(std::map<std::string, int64_t> &updated) {
    using namespace juce;
    using namespace juce;
    File d;
    Dimension dim;
    {
      ScopedLock lk(fSection);
      d = fDirectory;
      dim = fDim;
    }

    if (!d.exists()) {
      return;
    }

    File const root = DimensionDirectory(d, dim);

    std::map<std::string, int64_t> copy(std::find_if(updated.begin(), updated.end(), [root](auto it) {
                                          std::string s = it.first;
                                          String path(s);
                                          File f(path);
                                          return f.getParentDirectory().getFullPathName() == root.getFullPathName();
                                        }),
                                        updated.end());

    RangedDirectoryIterator it(DimensionDirectory(d, dim), false, "*.mca");
    std::vector<File> files;
    for (DirectoryEntry entry : it) {
      File f = entry.getFile();
      auto r = mcfile::je::Region::MakeRegion(PathFromFile(f));
      if (!r) {
        continue;
      }
      Time modified = f.getLastModificationTime();
      std::string fullpath = f.getFullPathName().toStdString();

      auto j = copy.find(fullpath);
      if (j == copy.end()) {
        copy[fullpath] = modified.toMilliseconds();
      } else {
        if (j->second < modified.toMilliseconds()) {
          copy[fullpath] = modified.toMilliseconds();
          files.push_back(f);
        }
      }
    }

    copy.swap(updated);

    if (!files.empty()) {
      fDelegate->regionUpdateCheckerDidDetectRegionFileUpdate(files, dim);
    }
  }

private:
  juce::CriticalSection fSection;
  juce::File fDirectory;
  Dimension fDim;
  Delegate *const fDelegate;
};

} // namespace mcview
