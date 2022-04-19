#pragma once

#include <juce_gui_extra/juce_gui_extra.h>

enum class Dimension : int {
    Overworld = 0,
    TheNether = -1,
    TheEnd    = 1,
};

static inline juce::File DimensionDirectory(juce::File directory, Dimension dim)
{
    if (dim == Dimension::Overworld) {
        return directory.getChildFile("region");
    }
    return directory.getChildFile(juce::String::formatted("DIM%d", (int)dim)).getChildFile("region");
}
