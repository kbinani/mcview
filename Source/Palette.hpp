#pragma once

namespace mcview {

class Palette {
  Palette() = delete;

public:
  static std::optional<juce::Colour> ColorFromId(mcfile::blocks::BlockId id);
};

} // namespace mcview
