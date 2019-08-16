#pragma once

#include "../JuceLibraryCode/JuceHeader.h"

class GraphicsHelper
{
public:
    static void DrawText(Graphics &g, String const& text, float x, float y, float width, float height,
                         Justification justificationType, bool const useEllipsesIfTooBig = true);

    static void DrawText(Graphics &g, String const& text, Rectangle<float> area,
                         Justification justificationType, bool const useEllipsesIfTooBig = true)
    {
        DrawText(g, text, area.getX(), area.getY(), area.getWidth(), area.getHeight(), justificationType, useEllipsesIfTooBig);
    }

    static void DrawFittedText(Graphics &g, String const& text, float x, float y, float width, float height,
                               Justification justification,
                               int const maximumNumberOfLines,
                               float const minimumHorizontalScale = 0.0f);

    static void DrawFittedText(Graphics &g, String const& text, Rectangle<float> area,
                               Justification justification,
                               int const maximumNumberOfLines,
                               float const minimumHorizontalScale = 0.0f)
    {
        DrawFittedText(g, text, area.getX(), area.getY(), area.getWidth(), area.getHeight(), justification, maximumNumberOfLines, minimumHorizontalScale);
    }

    static Font FallbackFont(Font font, String text);

private:
    GraphicsHelper() = delete;
};
