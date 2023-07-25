#pragma once

namespace mcview {

struct Directory {
  juce::File fDirectory;
  Edition fEdition;

  bool operator==(Directory const &other) const {
    return fDirectory == other.fDirectory && fEdition == other.fEdition;
  }
};

} // namespace mcview
