#include "GraphicsHelper.h"
#include "defer.h"

using namespace juce;

Font GraphicsHelper::FallbackFont(Font font, String text) {
  if (text.containsOnly(" !\"#$%&'()*+,-./0123456789:;<=>?@ABCDEFGHIJKLMNOPQRSTUVWXYZ[\\]^_`abcdefghijklmnopqrstuvwxyz{|}~")) {
    return font;
  }

  float height = font.getHeight();
  String name;
#if JUCE_WINDOWS
  name = "Yu Gothic UI";
  height *= 1.2f;
#else
  name = "Hiragino Kaku Gothic Pro";
  height *= 0.86f;
#endif
  Font f(name, height, font.getStyleFlags());
  return f;
}

void GraphicsHelper::DrawText(Graphics &g, String const &text, float x, float y, float width, float height,
                              Justification justificationType, bool const useEllipsesIfTooBig) {
  g.saveState();
  defer {
    g.restoreState();
  };
  Font f = FallbackFont(g.getCurrentFont(), text);
  g.setFont(f);
  g.drawText(text, x, y, width, height, justificationType, useEllipsesIfTooBig);
}

void GraphicsHelper::DrawFittedText(Graphics &g, String const &text, float x, float y, float width, float height,
                                    Justification justification,
                                    int const maximumNumberOfLines,
                                    float const minimumHorizontalScale) {
  g.saveState();
  defer {
    g.restoreState();
  };
  Font f = FallbackFont(g.getCurrentFont(), text);
  g.setFont(f);
  g.drawFittedText(text, x, y, width, height, justification, maximumNumberOfLines, minimumHorizontalScale);
}
