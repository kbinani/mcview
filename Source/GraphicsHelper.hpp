#pragma once

class GraphicsHelper {
public:
  static void DrawText(juce::Graphics &g, juce::String const &text, float x, float y, float width, float height,
                       juce::Justification justificationType, bool const useEllipsesIfTooBig = true) {
    g.saveState();
    defer {
      g.restoreState();
    };
    juce::Font f = FallbackFont(g.getCurrentFont(), text);
    g.setFont(f);
    g.drawText(text, x, y, width, height, justificationType, useEllipsesIfTooBig);
  }

  static void DrawText(juce::Graphics &g, juce::String const &text, juce::Rectangle<float> area,
                       juce::Justification justificationType, bool const useEllipsesIfTooBig = true) {
    DrawText(g, text, area.getX(), area.getY(), area.getWidth(), area.getHeight(), justificationType, useEllipsesIfTooBig);
  }

  static void DrawFittedText(juce::Graphics &g, juce::String const &text, float x, float y, float width, float height,
                             juce::Justification justification,
                             int const maximumNumberOfLines,
                             float const minimumHorizontalScale = 0.0f) {
    g.saveState();
    defer {
      g.restoreState();
    };
    juce::Font f = FallbackFont(g.getCurrentFont(), text);
    g.setFont(f);
    g.drawFittedText(text, x, y, width, height, justification, maximumNumberOfLines, minimumHorizontalScale);
  }

  static void DrawFittedText(juce::Graphics &g, juce::String const &text, juce::Rectangle<float> area,
                             juce::Justification justification,
                             int const maximumNumberOfLines,
                             float const minimumHorizontalScale = 0.0f) {
    DrawFittedText(g, text, area.getX(), area.getY(), area.getWidth(), area.getHeight(), justification, maximumNumberOfLines, minimumHorizontalScale);
  }

  static juce::Font FallbackFont(juce::Font font, juce::String text) {
    if (text.containsOnly(" !\"#$%&'()*+,-./0123456789:;<=>?@ABCDEFGHIJKLMNOPQRSTUVWXYZ[\\]^_`abcdefghijklmnopqrstuvwxyz{|}~")) {
      return font;
    }

    float height = font.getHeight();
    juce::String name;
#if JUCE_WINDOWS
    name = "Yu Gothic UI";
    height *= 1.2f;
#else
    name = "Hiragino Kaku Gothic Pro";
    height *= 0.86f;
#endif
    juce::Font f(name, height, font.getStyleFlags());
    return f;
  }

private:
  GraphicsHelper() = delete;
};
