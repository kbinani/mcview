#pragma once

#include "../JuceLibraryCode/JuceHeader.h"

class SettingsComponent : public Component {
public:
    std::function<void(float)> onWaterAbsorptionCoefficientChanged;

    static float constexpr kDefaultWaterAbsorptionCoefficient = 0.02f;
    
public:
    SettingsComponent();
    
    void paint(Graphics &g) override;
    void resized() override;

private:
    ScopedPointer<Slider> fWaterAbsorptionCoefficient;
    ScopedPointer<Label> fWaterAbsorptionCoefficientLabel;
};
