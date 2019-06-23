#pragma once

#include "../JuceLibraryCode/JuceHeader.h"

class SettingsComponent : public Component {
public:
    std::function<void(float)> onWaterAbsorptionCoefficientChanged;
    std::function<void(bool)> onWaterTranslucentChanged;

    static float constexpr kDefaultWaterAbsorptionCoefficient = 0.02f;
    
public:
    SettingsComponent();
    
    void paint(Graphics &g) override;
    void resized() override;

private:
    ScopedPointer<Slider> fWaterAbsorptionCoefficient;
    ScopedPointer<Label> fWaterAbsorptionCoefficientLabel;

    ScopedPointer<ToggleButton> fTranslucentWater;
};
