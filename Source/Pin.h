#pragma once

#include "../JuceLibraryCode/JuceHeader.h"
#include "Dimension.h"

class Pin {
public:
    static bool Parse(var v, Pin &dest);
    var toVar() const;

public:
    int fX;
    int fZ;
    String fMessage;
    Dimension fDim;
};
