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

class WorldData {
public:
    void save(File path) const;
    
    static WorldData Load(File path);
    static File WorldDataPath(File worldDirectory);

public:
    std::vector<Pin> fPins;
};
