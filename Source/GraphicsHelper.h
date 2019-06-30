#pragma once

#include "../JuceLibraryCode/JuceHeader.h"

class GraphicsHelper
{
public:
    static void DrawText(Graphics &g, String const& text, int x, int y, int width, int height,
                         Justification justificationType, bool const useEllipsesIfTooBig = true);
    
    static void DrawFittedText(Graphics &g, String const& text, int x, int y, int width, int height,
                               Justification justification,
                               int const maximumNumberOfLines,
                               float const minimumHorizontalScale = 0.0f);

private:
    GraphicsHelper() = delete;
};
