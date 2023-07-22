#pragma once

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

class RegionToTexture {
  RegionToTexture() = delete;

public:
  static void Load(mcfile::je::Region const &region, juce::ThreadPoolJob *job, Dimension dim, std::function<void(juce::PixelARGB *)> completion);
  static juce::File CacheFile(juce::File const &file);

public:
  static std::map<mcfile::blocks::BlockId, juce::Colour> const kBlockToColor;

  static juce::Colour const kDefaultOceanColor;
  static std::map<Biome, juce::Colour> const kOceanToColor;

  static juce::Colour const kDefaultFoliageColor;
  static std::map<Biome, juce::Colour> const kFoliageToColor;
};
