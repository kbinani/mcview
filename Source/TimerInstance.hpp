#pragma once

#include <juce_gui_extra/juce_gui_extra.h>

class TimerInstance : public juce::Timer {
public:
  TimerInstance() = default;

  std::function<void(TimerInstance &timer)> fTimerCallback;

private:
  void timerCallback() override {
    if (!fTimerCallback) {
      return;
    }
    fTimerCallback(*this);
  }
};
