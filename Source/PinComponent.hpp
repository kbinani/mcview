#pragma once

#include "Pin.hpp"
#include <juce_gui_extra/juce_gui_extra.h>

class PinComponent : public juce::Component {
public:
  explicit PinComponent(std::shared_ptr<Pin> const &pin);

  void paint(juce::Graphics &g) override;
  void mouseDown(juce::MouseEvent const &) override;
  void mouseUp(juce::MouseEvent const &) override;
  void mouseDoubleClick(juce::MouseEvent const &) override;
  void mouseDrag(juce::MouseEvent const &) override;

  void updatePinPosition(juce::Point<float> pos);
  bool isPresenting(std::shared_ptr<Pin> const &p) const {
    return p.get() == fPin.get();
  }
  juce::Point<float> getMapCoordinate() const {
    return juce::Point<float>(fPin->fX + 0.5f, fPin->fZ - 0.5f);
  }
  Dimension getDimension() const {
    return fPin->fDim;
  }

public:
  std::function<void(std::shared_ptr<Pin>, juce::Point<int> screenPos)> onRightClick;
  std::function<void(std::shared_ptr<Pin>, juce::Point<int> screenPos)> onDoubleClick;
  std::function<void(std::shared_ptr<Pin>, juce::Point<int> screenPos)> onDrag;
  std::function<void(std::shared_ptr<Pin>)> onDragEnd;

private:
  std::shared_ptr<Pin> fPin;
  juce::Point<int> fMouseDownToPinOffset;

  JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(PinComponent)
};
