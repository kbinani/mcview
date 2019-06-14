#pragma once
#include "../JuceLibraryCode/JuceHeader.h"
#include "minecraft-file.hpp"
#include "RegionToTexture.h"
#include "RegionTextureCache.h"

class RegionToTexture : public ThreadPoolJob {
public:
    RegionToTexture(File const& mcaFile, Region region);
    ~RegionToTexture();
    ThreadPoolJob::JobStatus runJob() override;
    
    static void Load(mcfile::Region const& region, std::function<void(PixelARGB *, uint8 *)> completion);
    
public:
    File const fRegionFile;
    Region const fRegion;
    ScopedPointer<PixelARGB> fPixels;
    ScopedPointer<uint8> fHeightmap;
};
