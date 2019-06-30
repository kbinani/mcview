#pragma once

#include "../JuceLibraryCode/JuceHeader.h"
#include "Settings.h"

class SettingsComponent : public Component {
public:
    std::function<void(float)> onWaterAbsorptionCoefficientChanged;
    std::function<void(bool)> onWaterTranslucentChanged;
    std::function<void(bool)> onBiomeEnableChanged;
    std::function<void(int)> onBiomeBlendChanged;

    static float const kDefaultWaterAbsorptionCoefficient;
    static float const kMaxWaterAbsorptionCoefficient;
    static float const kMinWaterAbsorptionCoefficient;
    
    static int const kDefaultBiomeBlend;
    static int const kMaxBiomeBlend;
    static int const kMinBiomeBlend;
    
public:
    explicit SettingsComponent(Settings const& settings);
    
    void paint(Graphics &g) override;
    void resized() override;

private:
    ScopedPointer<GroupComponent> fGroupWater;
    ScopedPointer<GroupComponent> fGroupBiome;
};
