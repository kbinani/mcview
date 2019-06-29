#pragma once
#include "../JuceLibraryCode/JuceHeader.h"
#include "minecraft-file.hpp"
#include "RegionToTexture.h"
#include "Region.h"

class RegionToTexture : public ThreadPoolJob {
public:
    RegionToTexture(File const& mcaFile, Region region);
    ~RegionToTexture();
    ThreadPoolJob::JobStatus runJob() override;
    
    static void Load(mcfile::Region const& region, ThreadPoolJob* job, std::function<void(PixelARGB *)> completion);
    
public:
    File const fRegionFile;
    Region const fRegion;
    ScopedPointer<PixelARGB> fPixels;
    
    static std::map<mcfile::blocks::BlockId, Colour> const kBlockToColor;

    static int constexpr kBlockIdOffset = 100;
    static_assert(mcfile::biomes::minecraft::minecraft_max_biome_id < kBlockIdOffset, "");

    static Colour const kDefaultOceanColor;
    static std::map<mcfile::biomes::BiomeId, Colour> const kOceanToColor;

    static Colour const kDefaultFoliageColor;
    static std::map<mcfile::biomes::BiomeId, Colour> const kFoliageToColor;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(RegionToTexture);
};
