#pragma once

namespace mcview {

enum class Dimension : int {
  Overworld = 0,
  TheNether = -1,
  TheEnd = 1,
};

static inline mcfile::Dimension DimensionFromDimension(Dimension d) {
  switch (d) {
  case Dimension::TheEnd:
    return mcfile::Dimension::End;
  case Dimension::TheNether:
    return mcfile::Dimension::Nether;
  case Dimension::Overworld:
  default:
    return mcfile::Dimension::Overworld;
  }
}

static inline juce::File DimensionDirectory(juce::File directory, Dimension dim) {
  if (dim == Dimension::Overworld) {
    return directory.getChildFile("region");
  }
  return directory.getChildFile(juce::String::formatted("DIM%d", (int)dim)).getChildFile("region");
}

} // namespace mcview
