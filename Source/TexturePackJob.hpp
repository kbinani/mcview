#pragma once

namespace mcview {

class TexturePackJob : public ThreadPoolJob {
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

  TexturePackJob(juce::String name, Delegate *delegate) : ThreadPoolJob(name), fDelegate(delegate) {}
  ~TexturePackJob() override = default;

protected:
  static bool LoadCache(std::unique_ptr<juce::PixelARGB[]> &pixels, std::optional<int64_t> timestamp, juce::File file) {
    juce::FileInputStream stream(file);
    if (!stream.openedOk()) {
      return false;
    }
    juce::GZIPDecompressorInputStream ungzip(stream);
    int64_t cachedModificationTime = 0;
    if (ungzip.read(&cachedModificationTime, sizeof(cachedModificationTime)) != sizeof(cachedModificationTime)) {
      return false;
    }
    if (!timestamp || (timestamp && cachedModificationTime >= *timestamp)) {
      pixels.reset(new juce::PixelARGB[512 * 512]);
      int expectedBytes = sizeof(juce::PixelARGB) * 512 * 512;
      if (ungzip.read(pixels.get(), expectedBytes) != expectedBytes) {
        return false;
      }
    }
    return true;
  }

  static void StoreCache(juce::PixelARGB const *pixels, int64_t timestamp, juce::File file) {
    auto out = std::make_unique<juce::FileOutputStream>(file);
    if (!out->openedOk()) {
      return;
    }
    if (!out->setPosition(0)) {
      return;
    }
    if (auto ret = out->truncate(); ret.failed()) {
      return;
    }
    juce::GZIPCompressorOutputStream gzip(*out, 9);
    if (!gzip.write(&timestamp, sizeof(timestamp))) {
      return;
    }
    if (!gzip.write(pixels, sizeof(juce::PixelARGB) * 512 * 512)) {
      return;
    }
  }

  static juce::File CacheFile(juce::File const &worldDirectory, Dimension dim, Region region) {
    using namespace juce;
#if JUCE_WINDOWS
    File tmp = File::getSpecialLocation(File::SpecialLocationType::tempDirectory).getChildFile("mcview").getChildFile("cache");
#else
    File tmp = File::getSpecialLocation(File::SpecialLocationType::tempDirectory).getChildFile("cache");
#endif
    if (!tmp.exists()) {
      tmp.createDirectory();
    }
    String hashSource = worldDirectory.getFullPathName();
    hashSource += "\n" + String(static_cast<int>(dim));
    hashSource += "\n" + String(region.first) + "_" + String(region.second);
    String hash = String("v5.") + String(hashSource.hashCode64());
    File dir = tmp.getChildFile(hash);
    if (!dir.exists()) {
      dir.createDirectory();
    }
    return dir.getChildFile(String("r.") + String(region.first) + "." + String(region.second) + String(".gz"));
  }

protected:
  Delegate *const fDelegate;
};

} // namespace mcview
