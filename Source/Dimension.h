#pragma once

#include "../JuceLibraryCode/JuceHeader.h"

enum class Dimension : int {
    Overworld = 0,
    TheNether = -1,
    TheEnd    = 1,
};

static inline File DimensionDirectory(File directory, Dimension dim)
{
    if (dim == Dimension::Overworld) {
        return directory.getChildFile("region");
    }
    return directory.getChildFile(String::formatted("DIM%d", (int)dim)).getChildFile("region");
}
