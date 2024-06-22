#pragma once

namespace mcview {

class PinComponent : public juce::Component {
  static int constexpr pinHeadRadius = 6;
  static int constexpr pinNameFontSize = 14;
  static int constexpr stemLength = 16;
  static juce::Point<float> constexpr pinHilightOffset = juce::Point<float>(-2, -2);
  static int constexpr pinHeadHilightRadius = 3;
  static juce::Point<float> constexpr pinHeadShadowOffset = juce::Point<float>(3, 3);
  static float constexpr pinHeadShadowAlpha = 0.5f;
  static float constexpr stemThickness = 2;

public:
  explicit PinComponent(std::shared_ptr<Pin> const &pin)
      : fPin(pin) {
    int const height = stemLength + pinHeadRadius * 2;
    juce::Point<float> pos(pinHeadRadius, height);
    juce::Font font(juce::FontOptions().withHeight(pinNameFontSize));
    juce::Rectangle<float> nameBounds = PinNameBounds(*fPin, font, pos);
    int const width = nameBounds.getRight();
    setSize(width, juce::jmax(height, height - (int)nameBounds.getY()));
  }

  void paint(juce::Graphics &g) override {
    static juce::Colour const pinHeadColour = juce::Colours::red;
    static juce::Colour const pinHeadColourNether = juce::Colours::blue;
    static juce::Colour const stemColour = juce::Colours::white;

    g.saveState();
    defer {
      g.restoreState();
    };

    juce::Point<float> pos(pinHeadRadius, getHeight());
    juce::Font font(juce::FontOptions().withHeight(pinNameFontSize));

    g.setColour(juce::Colours::black.withAlpha(pinHeadShadowAlpha));
    g.fillEllipse(pos.x + pinHeadShadowOffset.x - pinHeadRadius, pos.y + pinHeadShadowOffset.y - pinHeadRadius - stemLength, pinHeadRadius * 2, pinHeadRadius * 2);

    g.setColour(stemColour);
    g.drawLine(pos.x, pos.y - stemLength, pos.x, pos.y, stemThickness);

    juce::Colour const headColor = fPin->fDim == Dimension::TheNether ? pinHeadColourNether : pinHeadColour;
    g.setColour(headColor);
    g.fillEllipse(pos.x - pinHeadRadius, pos.y - pinHeadRadius - stemLength, pinHeadRadius * 2, pinHeadRadius * 2);

    g.setColour(headColor.brighter().brighter());
    g.fillEllipse(pos.x + pinHilightOffset.x - pinHeadHilightRadius, pos.y + pinHilightOffset.y - pinHeadHilightRadius - stemLength, pinHeadHilightRadius * 2, pinHeadHilightRadius * 2);

    g.setColour(juce::Colours::black.withAlpha(0.5f));
    auto stringBounds = PinNameBounds(*fPin, font, pos);
    g.fillRect(stringBounds);
    g.setColour(juce::Colours::white);
    g.setFont(pinNameFontSize);
    g.drawText(fPin->fMessage, stringBounds, juce::Justification::centred);
  }

  void mouseDown(juce::MouseEvent const &e) override {
    juce::Point<int> localPinPos(pinHeadRadius, getHeight());
    fMouseDownToPinOffset = juce::Point<int>(e.x - localPinPos.x, e.y - localPinPos.y);
  }

  void mouseUp(juce::MouseEvent const &e) override {
    if (e.mods.isRightButtonDown()) {
      if (onRightClick && e.getNumberOfClicks() == 1 && e.mouseWasClicked()) {
        onRightClick(fPin, e.getScreenPosition());
      }
    } else if (e.mouseWasDraggedSinceMouseDown()) {
      if (onDragEnd) {
        onDragEnd(fPin);
      }
    }
  }

  void mouseDoubleClick(juce::MouseEvent const &e) override {
    if (onDoubleClick) {
      onDoubleClick(fPin, e.getScreenPosition());
    }
  }

  void mouseDrag(juce::MouseEvent const &e) override {
    if (e.mods.isRightButtonDown()) {
      return;
    }
    if (onDrag) {
      juce::Point<int> pos(e.getScreenX() - fMouseDownToPinOffset.x, e.getScreenY() - fMouseDownToPinOffset.y);
      onDrag(fPin, pos);
    }
  }

  void updatePinPosition(juce::Point<float> pos) {
    fPos = pos;
    updateSize();
  }

  void updateSize() {
    int const h = stemLength + pinHeadRadius * 2;
    juce::Font font(juce::FontOptions().withHeight(pinNameFontSize));
    juce::Rectangle<float> nameBounds = PinNameBounds(*fPin, font, fPos);
    int const left = fPos.x - pinHeadRadius;
    int const width = nameBounds.getRight() - left;
    int const height = juce::jmax(h, h - (int)nameBounds.getY());
    setBounds(fPos.x - pinHeadRadius, fPos.y - height, width, height);
  }

  bool isPresenting(std::shared_ptr<Pin> const &p) const {
    return p.get() == fPin.get();
  }

  juce::Point<float> getMapCoordinate() const {
    return juce::Point<float>(fPin->fX + 0.5f, fPin->fZ - 0.5f);
  }

  Dimension getDimension() const {
    return fPin->fDim;
  }

private:
  static juce::Rectangle<float> PinNameBounds(Pin pin, juce::Font font, juce::Point<float> pos) {
    int const pad = 4;
    float stringWidth = font.getStringWidthFloat(pin.fMessage);
    juce::Rectangle<float> stringBounds(pos.x + pinHeadRadius + pad, pos.y - stemLength - font.getHeight() / 2 - pad, pad + stringWidth + pad, pad + font.getHeight() + pad);
    return stringBounds;
  }

public:
  std::function<void(std::shared_ptr<Pin>, juce::Point<int> screenPos)> onRightClick;
  std::function<void(std::shared_ptr<Pin>, juce::Point<int> screenPos)> onDoubleClick;
  std::function<void(std::shared_ptr<Pin>, juce::Point<int> screenPos)> onDrag;
  std::function<void(std::shared_ptr<Pin>)> onDragEnd;

private:
  std::shared_ptr<Pin> fPin;
  juce::Point<int> fMouseDownToPinOffset;
  juce::Point<float> fPos;

  JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(PinComponent)
};

} // namespace mcview
