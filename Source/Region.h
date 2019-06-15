#pragma once

#include "../JuceLibraryCode/JuceHeader.h"
#include <map>

using Region = std::pair<int, int>;

static inline Region MakeRegion(int x, int z) {
    return std::make_pair(x, z);
}

static inline String RegionFileName(Region region) {
    return String::formatted("r.%d.%d.mca", region.first, region.second);
}

