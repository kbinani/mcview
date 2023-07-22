#pragma once

#include <juce_gui_extra/juce_gui_extra.h>

class GraphicsHelper {
public:
  static void DrawText(juce::Graphics &g, juce::String const &text, float x, float y, float width, float height,
                       juce::Justification justificationType, bool const useEllipsesIfTooBig = true);

  static void DrawText(juce::Graphics &g, juce::String const &text, juce::Rectangle<float> area,
                       juce::Justification justificationType, bool const useEllipsesIfTooBig = true) {
    DrawText(g, text, area.getX(), area.getY(), area.getWidth(), area.getHeight(), justificationType, useEllipsesIfTooBig);
  }

  static void DrawFittedText(juce::Graphics &g, juce::String const &text, float x, float y, float width, float height,
                             juce::Justification justification,
                             int const maximumNumberOfLines,
                             float const minimumHorizontalScale = 0.0f);

  static void DrawFittedText(juce::Graphics &g, juce::String const &text, juce::Rectangle<float> area,
                             juce::Justification justification,
                             int const maximumNumberOfLines,
                             float const minimumHorizontalScale = 0.0f) {
    DrawFittedText(g, text, area.getX(), area.getY(), area.getWidth(), area.getHeight(), justification, maximumNumberOfLines, minimumHorizontalScale);
  }

  static juce::Font FallbackFont(juce::Font font, juce::String text);

private:
  GraphicsHelper() = delete;
};
