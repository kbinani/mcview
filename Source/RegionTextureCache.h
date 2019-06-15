#pragma once

#include "../JuceLibraryCode/JuceHeader.h"
#include <map>

using Region = std::pair<int, int>;

static inline Region MakeRegion(int x, int z) {
    return std::make_pair(x, z);
}

static inline String RegionFileName(Region region) {
    return String::formatted("r.%d.%d.mca", region.first, region.second);
}

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
