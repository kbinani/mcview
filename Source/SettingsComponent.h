#pragma once

#include "../JuceLibraryCode/JuceHeader.h"

class SettingsComponent : public Component {
public:
    std::function<void(float)> onWaterAbsorptionCoefficientChanged;
    std::function<void(bool)> onWaterTranslucentChanged;
    std::function<void(bool)> onBiomeEnableChanged;
    std::function<void(int)> onBiomeBlendChanged;

    static float constexpr kDefaultWaterAbsorptionCoefficient = 0.02f;
    
public:
    SettingsComponent();
    
    void paint(Graphics &g) override;
    void resized() override;

private:
    ScopedPointer<GroupComponent> fGroupWater;
    ScopedPointer<GroupComponent> fGroupBiome;
};
