#pragma once

#include "../JuceLibraryCode/JuceHeader.h"
#include "Pin.h"

class WorldData {
public:
    void save(File path) const;
    
    static WorldData Load(File path);
    static File WorldDataPath(File worldDirectory);

public:
    std::vector<std::shared_ptr<Pin>> fPins;
};
