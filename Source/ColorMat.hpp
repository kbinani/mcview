#pragma once

namespace mcview {

class ColorMat : public juce::Component {
public:
  explicit ColorMat(juce::Colour color) : fColor(color) {}

  void paint(juce::Graphics &g) override {
    g.fillAll(fColor);
  }

private:
  juce::Colour fColor;
};

} // namespace mcview
