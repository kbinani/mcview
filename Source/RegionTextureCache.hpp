#pragma once

namespace mcview {

class RegionTextureCache {
public:
  RegionTextureCache(juce::File worldDirectory, Dimension dim, Region region)
      : fWorldDirectory(worldDirectory), fDimension(dim), fRegion(region) {
  }

  void load(juce::PixelARGB *pixels) {
    std::unique_ptr<juce::OpenGLTexture> p(new juce::OpenGLTexture());
    p->loadARGB(pixels, 512, 512);
    fTexture.reset(p.release());

    fLoadTime = juce::Time::getCurrentTime();
  }

public:
  juce::File const fWorldDirectory;
  Dimension const fDimension;
  Region const fRegion;
  std::unique_ptr<juce::OpenGLTexture> fTexture;
  juce::Time fLoadTime;

  JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(RegionTextureCache);
};

} // namespace mcview
