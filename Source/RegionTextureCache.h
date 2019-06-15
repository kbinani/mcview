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
    ScopedPointer<OpenGLTexture> fTexture;
    Time fLoadTime;
};
