#pragma once

#include <juce_gui_extra/juce_gui_extra.h>
#include "Settings.h"

class SettingsComponent : public juce::Component {
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
    
    void paint(juce::Graphics &g) override;
    void resized() override;

private:
    std::unique_ptr<juce::GroupComponent> fGroupWater;
    std::unique_ptr<juce::GroupComponent> fGroupBiome;
    std::unique_ptr<juce::GroupComponent> fGroupPin;
    std::unique_ptr<juce::HyperlinkButton> fAboutButton;
};
