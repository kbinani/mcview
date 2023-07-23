#pragma once

namespace mcview {

class Palette {
  Palette() = delete;

public:
  static std::optional<juce::Colour> ColorFromId(mcfile::blocks::BlockId id);
  static std::optional<juce::Colour> JavaColorFromId(mcfile::blocks::BlockId id);

  static bool IsWater(mcfile::blocks::BlockId id) {
    using namespace mcfile::blocks::minecraft;
    return id == water || id == bubble_column || id == kelp || id == kelp_plant || id == seagrass || id == tall_seagrass;
  }

  // static void ResearchJava(std::filesystem::path);
};

} // namespace mcview
