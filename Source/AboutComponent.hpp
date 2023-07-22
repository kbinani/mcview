#pragma once

#include <juce_gui_extra/juce_gui_extra.h>

class AboutComponent : public juce::Component {
public:
  AboutComponent();
  void paint(juce::Graphics &g) override;

private:
  std::unique_ptr<juce::Drawable> fLogo;
  juce::Array<std::unique_ptr<Component>> fLines;
};
