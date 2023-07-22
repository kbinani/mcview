#pragma once

namespace mcview {

class LookAndFeel : public juce::LookAndFeel_V4 {
  void drawButtonText(juce::Graphics &g, juce::TextButton &button, bool shouldDrawButtonAsHighlighted, bool shouldDrawButtonAsDown) override {
    juce::LookAndFeel_V4::drawButtonText(g, button, shouldDrawButtonAsHighlighted, shouldDrawButtonAsDown);
  }

  void drawLabel(juce::Graphics &g, juce::Label &label) override {
    using namespace juce;

    g.fillAll(label.findColour(Label::backgroundColourId));

    if (!label.isBeingEdited()) {
      auto alpha = label.isEnabled() ? 1.0f : 0.5f;
      const Font font(getLabelFont(label));

      g.setColour(label.findColour(Label::textColourId).withMultipliedAlpha(alpha));
      g.setFont(font);

      juce::Rectangle<float> textArea = getLabelBorderSize(label).subtractedFrom(label.getLocalBounds()).toFloat();

      GraphicsHelper::DrawFittedText(g, label.getText(), textArea, label.getJustificationType(),
                                     jmax(1, (int)(textArea.getHeight() / font.getHeight())),
                                     label.getMinimumHorizontalScale());

      g.setColour(label.findColour(Label::outlineColourId).withMultipliedAlpha(alpha));
    } else if (label.isEnabled()) {
      g.setColour(label.findColour(Label::outlineColourId));
    }

    g.drawRect(label.getLocalBounds());
  }

  void drawGroupComponentOutline(juce::Graphics &g, int width, int height, const juce::String &text, const juce::Justification &position, juce::GroupComponent &group) override {
    using namespace juce;

    const float textH = 15.0f;
    const float indent = 3.0f;
    const float textEdgeGap = 4.0f;
    auto cs = 5.0f;

    Font f(textH);

    Path p;
    auto x = indent;
    auto y = f.getAscent() - 3.0f;
    auto w = jmax(0.0f, width - x * 2.0f);
    auto h = jmax(0.0f, height - y - indent);
    cs = jmin(cs, w * 0.5f, h * 0.5f);
    auto cs2 = 2.0f * cs;

    auto textW = text.isEmpty() ? 0 : jlimit(0.0f, jmax(0.0f, w - cs2 - textEdgeGap * 2), f.getStringWidth(text) + textEdgeGap * 2.0f);
    auto textX = cs + textEdgeGap;

    if (position.testFlags(Justification::horizontallyCentred))
      textX = cs + (w - cs2 - textW) * 0.5f;
    else if (position.testFlags(Justification::right))
      textX = w - cs - textW - textEdgeGap;

    p.startNewSubPath(x + textX + textW, y);
    p.lineTo(x + w - cs, y);

    p.addArc(x + w - cs2, y, cs2, cs2, 0, MathConstants<float>::halfPi);
    p.lineTo(x + w, y + h - cs);

    p.addArc(x + w - cs2, y + h - cs2, cs2, cs2, MathConstants<float>::halfPi, MathConstants<float>::pi);
    p.lineTo(x + cs, y + h);

    p.addArc(x, y + h - cs2, cs2, cs2, MathConstants<float>::pi, MathConstants<float>::pi * 1.5f);
    p.lineTo(x, y + cs);

    p.addArc(x, y, cs2, cs2, MathConstants<float>::pi * 1.5f, MathConstants<float>::twoPi);
    p.lineTo(x + textX, y);

    auto alpha = group.isEnabled() ? 1.0f : 0.5f;

    g.setColour(group.findColour(GroupComponent::outlineColourId)
                    .withMultipliedAlpha(alpha));

    g.strokePath(p, PathStrokeType(2.0f));

    g.setColour(group.findColour(GroupComponent::textColourId)
                    .withMultipliedAlpha(alpha));
    g.setFont(f);
    GraphicsHelper::DrawText(g, text,
                             roundToInt(x + textX), 0,
                             roundToInt(textW),
                             roundToInt(textH),
                             Justification::centred, true);
  }

  void drawToggleButton(juce::Graphics &g, juce::ToggleButton &button, bool shouldDrawButtonAsHighlighted, bool shouldDrawButtonAsDown) override {
    using namespace juce;

    auto fontSize = jmin(15.0f, button.getHeight() * 0.75f);
    auto tickWidth = fontSize * 1.1f;

    drawTickBox(g, button, 4.0f, (button.getHeight() - tickWidth) * 0.5f,
                tickWidth, tickWidth,
                button.getToggleState(),
                button.isEnabled(),
                shouldDrawButtonAsHighlighted,
                shouldDrawButtonAsDown);

    g.setColour(button.findColour(ToggleButton::textColourId));
    g.setFont(fontSize);

    if (!button.isEnabled())
      g.setOpacity(0.5f);

    GraphicsHelper::DrawFittedText(g, button.getButtonText(),
                                   button.getLocalBounds().withTrimmedLeft(roundToInt(tickWidth) + 10).withTrimmedRight(2).toFloat(),
                                   Justification::centredLeft, 10);
  }

  void drawPopupMenuItem(juce::Graphics &g, juce::Rectangle<int> const &area, bool isSeparator, bool isActive, bool isHighlighted, bool isTicked, bool hasSubMenu, juce::String const &text, juce::String const &shortcutKeyText, juce::Drawable const *icon, juce::Colour const *textColourToUse) override {
    using namespace juce;

    if (isSeparator) {
      auto r = area.reduced(5, 0);
      r.removeFromTop(roundToInt((r.getHeight() * 0.5f) - 0.5f));

      g.setColour(findColour(PopupMenu::textColourId).withAlpha(0.3f));
      g.fillRect(r.removeFromTop(1));
    } else {
      auto textColour = (textColourToUse == nullptr ? findColour(PopupMenu::textColourId)
                                                    : *textColourToUse);

      auto r = area.reduced(1);

      if (isHighlighted && isActive) {
        g.setColour(findColour(PopupMenu::highlightedBackgroundColourId));
        g.fillRect(r);

        g.setColour(findColour(PopupMenu::highlightedTextColourId));
      } else {
        g.setColour(textColour.withMultipliedAlpha(isActive ? 1.0f : 0.5f));
      }

      r.reduce(jmin(5, area.getWidth() / 20), 0);

      auto font = getPopupMenuFont();

      auto maxFontHeight = r.getHeight() / 1.3f;

      if (font.getHeight() > maxFontHeight)
        font.setHeight(maxFontHeight);

      g.setFont(font);

      auto iconArea = r.removeFromLeft(roundToInt(maxFontHeight)).toFloat();

      if (icon != nullptr) {
        icon->drawWithin(g, iconArea, RectanglePlacement::centred | RectanglePlacement::onlyReduceInSize, 1.0f);
        r.removeFromLeft(roundToInt(maxFontHeight * 0.5f));
      } else if (isTicked) {
        auto tick = getTickShape(1.0f);
        g.fillPath(tick, tick.getTransformToScaleToFit(iconArea.reduced(iconArea.getWidth() / 5, 0).toFloat(), true));
      }

      if (hasSubMenu) {
        auto arrowH = 0.6f * getPopupMenuFont().getAscent();

        auto x = static_cast<float>(r.removeFromRight((int)arrowH).getX());
        auto halfH = static_cast<float>(r.getCentreY());

        Path path;
        path.startNewSubPath(x, halfH - arrowH * 0.5f);
        path.lineTo(x + arrowH * 0.6f, halfH);
        path.lineTo(x, halfH + arrowH * 0.5f);

        g.strokePath(path, PathStrokeType(2.0f));
      }

      r.removeFromRight(3);
      GraphicsHelper::DrawFittedText(g, text, r.toFloat(), Justification::centredLeft, 1);

      if (shortcutKeyText.isNotEmpty()) {
        auto f2 = font;
        f2.setHeight(f2.getHeight() * 0.75f);
        f2.setHorizontalScale(0.95f);
        g.setFont(f2);

        g.drawText(shortcutKeyText, r, Justification::centredRight, true);
      }
    }
  }

  void drawDocumentWindowTitleBar(juce::DocumentWindow &window, juce::Graphics &g,
                                  int w, int h, int titleSpaceX, int titleSpaceW,
                                  const juce::Image *icon, bool drawTitleTextOnLeft) override {
    using namespace juce;

    if (w * h == 0)
      return;

    auto isActive = window.isActiveWindow();

    g.setColour(getCurrentColourScheme().getUIColour(ColourScheme::widgetBackground));
    g.fillAll();

    Font f(h * 0.65f, Font::plain);
    Font font = GraphicsHelper::FallbackFont(f, window.getName());
    g.setFont(f);

    auto textW = font.getStringWidth(window.getName());
    auto iconW = 0;
    auto iconH = 0;

    if (icon != nullptr) {
      iconH = static_cast<int>(font.getHeight());
      iconW = icon->getWidth() * iconH / icon->getHeight() + 4;
    }

    textW = jmin(titleSpaceW, textW + iconW);
    auto textX = drawTitleTextOnLeft ? titleSpaceX
                                     : jmax(titleSpaceX, (w - textW) / 2);

    if (textX + textW > titleSpaceX + titleSpaceW)
      textX = titleSpaceX + titleSpaceW - textW;

    if (icon != nullptr) {
      g.setOpacity(isActive ? 1.0f : 0.6f);
      g.drawImageWithin(*icon, textX, (h - iconH) / 2, iconW, iconH,
                        RectanglePlacement::centred, false);
      textX += iconW;
      textW -= iconW;
    }

    if (window.isColourSpecified(DocumentWindow::textColourId) || isColourSpecified(DocumentWindow::textColourId))
      g.setColour(window.findColour(DocumentWindow::textColourId));
    else
      g.setColour(getCurrentColourScheme().getUIColour(ColourScheme::defaultText));

    GraphicsHelper::DrawText(g, window.getName(), textX, 0, textW, h, Justification::centredLeft, true);
  }
};

} // namespace mcview
