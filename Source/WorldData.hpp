#pragma once

#include "Pin.hpp"
#include <juce_gui_extra/juce_gui_extra.h>

class WorldData {
public:
  void save(juce::File path) const;

  static WorldData Load(juce::File path);
  static juce::File WorldDataPath(juce::File worldDirectory);

public:
  std::vector<std::shared_ptr<Pin>> fPins;
};
