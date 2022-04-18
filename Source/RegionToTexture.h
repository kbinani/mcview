#pragma once
#include "../JuceLibraryCode/JuceHeader.h"
#include "minecraft-file.hpp"
#include "RegionToTexture.h"
#include "Region.h"
#include "Dimension.h"

enum class Biome : uint8_t {
    Other = 0,
    Swamp,
    Ocean,
    LukewarmOcean,
    WarmOcean,
    ColdOcean,
    FrozenOcean,
    Badlands,
    
    max_Biome,
};

inline Biome ToBiome(mcfile::biomes::BiomeId b)
{
    switch (b) {
        case mcfile::biomes::minecraft::ocean:
        case mcfile::biomes::minecraft::deep_ocean:
            return Biome::Ocean;
        case mcfile::biomes::minecraft::lukewarm_ocean:
        case mcfile::biomes::minecraft::deep_lukewarm_ocean:
            return Biome::LukewarmOcean;
        case mcfile::biomes::minecraft::warm_ocean:
        case mcfile::biomes::minecraft::deep_warm_ocean:
            return Biome::WarmOcean;
        case mcfile::biomes::minecraft::cold_ocean:
        case mcfile::biomes::minecraft::deep_cold_ocean:
            return Biome::ColdOcean;
        case mcfile::biomes::minecraft::frozen_ocean:
        case mcfile::biomes::minecraft::deep_frozen_ocean:
            return Biome::FrozenOcean;
        case mcfile::biomes::minecraft::swamp:
        case mcfile::biomes::minecraft::swamp_hills:
            return Biome::Swamp;
        case mcfile::biomes::minecraft::badlands:
            return Biome::Badlands;
        default:
            return Biome::Other;
    }
}

class RegionToTexture : public ThreadPoolJob {
public:
    RegionToTexture(File const& mcaFile, Region region, Dimension dim, bool useCache);
    ~RegionToTexture();
    ThreadPoolJob::JobStatus runJob() override;
    
    static void Load(mcfile::je::Region const& region, ThreadPoolJob* job, Dimension dim, std::function<void(PixelARGB *)> completion);
    static File CacheFile(File const& file);
    
public:
    File const fRegionFile;
    Region const fRegion;
    Dimension const fDimension;
    std::unique_ptr<PixelARGB[]> fPixels;
    bool const fUseCache;
    
    static std::map<mcfile::blocks::BlockId, Colour> const kBlockToColor;

    static Colour const kDefaultOceanColor;
    static std::map<Biome, Colour> const kOceanToColor;

    static Colour const kDefaultFoliageColor;
    static std::map<Biome, Colour> const kFoliageToColor;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(RegionToTexture);
};
