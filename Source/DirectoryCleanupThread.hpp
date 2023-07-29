#pragma once

namespace mcview {

class DirectoryCleanupThread : public juce::Thread {
public:
  DirectoryCleanupThread() : juce::Thread("Directory cleanup thread") {
    auto dir = CacheDirectory();
    auto prefix = TexturePackJob::CacheDirPrefix();
    for (auto const &it : juce::RangedDirectoryIterator(dir, false, "*", juce::File::findDirectories, juce::File::FollowSymlinks::no)) {
      auto f = it.getFile();
      auto name = f.getFileName();
      if (!name.startsWith(prefix)) {
        fDirs.push_back(f);
      }
    }

    auto work = WorkingDirectory();
    for (auto const &it : juce::RangedDirectoryIterator(work, false, "*", juce::File::findDirectories, juce::File::FollowSymlinks::no)) {
      fDirs.push_back(it.getFile());
    }
  }

  void run() override {
    for (juce::File const &dir : fDirs) {
      if (threadShouldExit()) {
        break;
      }
      dir.deleteRecursively(false);
    }
  }

private:
  std::vector<juce::File> fDirs;
};

} // namespace mcview
