#pragma once

#include <juce_gui_extra/juce_gui_extra.h>
#include <juce_opengl/juce_opengl.h>
#include "Region.h"

class RegionTextureCache {
public:
    RegionTextureCache(Region region, juce::String file)
        : fRegion(region)
        , fFile(file)
    {
    }
    
    void load(juce::PixelARGB *pixels);

public:
    Region const fRegion;
    juce::String const fFile;
    std::unique_ptr<juce::OpenGLTexture> fTexture;
    juce::Time fLoadTime;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(RegionTextureCache);
};
