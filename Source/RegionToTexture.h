#pragma once
#include "Dimension.h"
#include "Region.h"
#include "RegionToTexture.h"
#include "minecraft-file.hpp"
#include <juce_gui_extra/juce_gui_extra.h>

enum class Biome : uint8_t {
  Other = 0,
  Swamp,
  Ocean,
  LukewarmOcean,
  WarmOcean,
  ColdOcean,
  Badlands,
  MangroveSwamp,

  max_Biome,
};

inline Biome ToBiome(mcfile::biomes::BiomeId b) {
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
  case mcfile::biomes::minecraft::frozen_ocean:
  case mcfile::biomes::minecraft::deep_frozen_ocean:
    return Biome::ColdOcean;
  case mcfile::biomes::minecraft::swamp:
  case mcfile::biomes::minecraft::swamp_hills:
    return Biome::Swamp;
  case mcfile::biomes::minecraft::badlands:
    return Biome::Badlands;
  case mcfile::biomes::minecraft::mangrove_swamp:
    return Biome::MangroveSwamp;
  default:
    return Biome::Other;
  }
}

class RegionToTexture : public juce::ThreadPoolJob {
public:
  class Result {
  public:
    Result(Region region, juce::File regionFile, juce::File worldDirectory, Dimension dimension) : fRegion(region), fRegionFile(regionFile), fWorldDirectory(worldDirectory), fDimension(dimension) {}

    Region const fRegion;
    juce::File const fRegionFile;
    std::unique_ptr<juce::PixelARGB[]> fPixels;
    juce::File const fWorldDirectory;
    Dimension const fDimension;
  };

  class Delegate {
  public:
    virtual ~Delegate() = default;
    virtual void regionToTextureDidFinishJob(std::shared_ptr<Result> result) = 0;
  };

  RegionToTexture(juce::File const &worldDirectory, juce::File const &mcaFile, Region region, Dimension dim, bool useCache, Delegate *delegate);
  ~RegionToTexture();
  ThreadPoolJob::JobStatus runJob() override;

  static void Load(mcfile::je::Region const &region, ThreadPoolJob *job, Dimension dim, std::function<void(juce::PixelARGB *)> completion);
  static juce::File CacheFile(juce::File const &file);

public:
  static std::map<mcfile::blocks::BlockId, juce::Colour> const kBlockToColor;

  static juce::Colour const kDefaultOceanColor;
  static std::map<Biome, juce::Colour> const kOceanToColor;

  static juce::Colour const kDefaultFoliageColor;
  static std::map<Biome, juce::Colour> const kFoliageToColor;

private:
  juce::File const fRegionFile;
  Region const fRegion;
  juce::File const fWorldDirectory;
  Dimension const fDimension;
  bool const fUseCache;
  Delegate *const fDelegate;

  JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(RegionToTexture)
};
