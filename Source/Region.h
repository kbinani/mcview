#pragma once

#include <juce_gui_extra/juce_gui_extra.h>
#include <map>

using Region = std::pair<int, int>;

static inline Region MakeRegion(int x, int z) {
    return std::make_pair(x, z);
}

static inline juce::String RegionFileName(Region region) {
    return juce::String::formatted("r.%d.%d.mca", region.first, region.second);
}

