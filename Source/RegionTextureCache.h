#pragma once

#include "../JuceLibraryCode/JuceHeader.h"
#include "Region.h"

class RegionTextureCache {
public:
    RegionTextureCache(Region region, String file)
        : fRegion(region)
        , fFile(file)
    {
    }
    
    void load(PixelARGB *pixels);

public:
    Region const fRegion;
    String const fFile;
    std::unique_ptr<OpenGLTexture> fTexture;
    Time fLoadTime;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(RegionTextureCache);
};
