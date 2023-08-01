#pragma once

namespace mcview {

class ImageButton : public juce::DrawableButton {
public:
  ImageButton(juce::String const &buttonName, juce::DrawableButton::ButtonStyle buttonStyle) : juce::DrawableButton(buttonName, buttonStyle) {
    setMouseCursor(juce::MouseCursor::PointingHandCursor);
  }

  void enablementChanged() override {
    juce::DrawableButton::enablementChanged();
    if (isEnabled()) {
      setMouseCursor(juce::MouseCursor::PointingHandCursor);
    } else {
      setMouseCursor(juce::MouseCursor::NormalCursor);
    }
  }
};

} // namespace mcview
