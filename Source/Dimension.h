#pragma once

#include "../JuceLibraryCode/JuceHeader.h"

enum class Dimension {
    Overworld,
    Nether,
    End,
};

static inline File DimensionDirectory(File directory, Dimension dim)
{
    switch (dim) {
        case Dimension::Overworld:
            return directory.getChildFile("region");
        case Dimension::Nether:
            return directory.getChildFile("DIM-1").getChildFile("region");
        case Dimension::End:
            return directory.getChildFile("DIM1").getChildFile("region");
    }
    assert(false);
    return directory;
}
