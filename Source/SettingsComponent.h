#pragma once

#include "../JuceLibraryCode/JuceHeader.h"
#include "Settings.h"

class SettingsComponent : public Component {
public:
    std::function<void(float)> onWaterOpticalDensityChanged;
    std::function<void(bool)> onWaterTranslucentChanged;
    std::function<void(bool)> onBiomeEnableChanged;
    std::function<void(int)> onBiomeBlendChanged;
    std::function<void(bool)> onShowPinChanged;

    static float const kDefaultWaterOpticalDensity;
    static float const kMaxWaterOpticalDensity;
    static float const kMinWaterOpticalDensity;
    
    static int const kDefaultBiomeBlend;
    static int const kMaxBiomeBlend;
    static int const kMinBiomeBlend;
    
public:
    explicit SettingsComponent(Settings const& settings);
    
    void paint(Graphics &g) override;
    void resized() override;

private:
    std::unique_ptr<GroupComponent> fGroupWater;
    std::unique_ptr<GroupComponent> fGroupBiome;
    std::unique_ptr<GroupComponent> fGroupPin;
    std::unique_ptr<HyperlinkButton> fAboutButton;
};
