#pragma once

#include "../JuceLibraryCode/JuceHeader.h"

class TimerInstance : public Timer
{
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
