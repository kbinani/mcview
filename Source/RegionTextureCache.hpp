#pragma once

class RegionTextureCache {
public:
  RegionTextureCache(Region region, juce::String file)
      : fRegion(region), fFile(file) {
  }

  void load(juce::PixelARGB *pixels) {
    std::unique_ptr<juce::OpenGLTexture> p(new juce::OpenGLTexture());
    p->loadARGB(pixels, 512, 512);
    fTexture.reset(p.release());

    fLoadTime = juce::Time::getCurrentTime();
  }

public:
  Region const fRegion;
  juce::String const fFile;
  std::unique_ptr<juce::OpenGLTexture> fTexture;
  juce::Time fLoadTime;

  JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(RegionTextureCache);
};
