#pragma once

#include "Dimension.h"
#include <juce_gui_extra/juce_gui_extra.h>

class Pin {
public:
  static bool Parse(juce::var v, Pin &dest);
  juce::var toVar() const;

public:
  int fX;
  int fZ;
  juce::String fMessage;
  Dimension fDim;
};
