#pragma once
#include "../JuceLibraryCode/JuceHeader.h"
#include "minecraft-file.hpp"

class RegionToTexture {
public:
    static void Load(mcfile::Region const& region, std::function<void(PixelARGB *, uint8 *)> completion);
};
