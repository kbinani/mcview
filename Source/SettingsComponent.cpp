#include "SettingsComponent.h"
#include "defer.h"

float const SettingsComponent::kDefaultWaterAbsorptionCoefficient = 0.02f;
float const SettingsComponent::kMaxWaterAbsorptionCoefficient = 0.04f;
float const SettingsComponent::kMinWaterAbsorptionCoefficient = 0.0f;

int const SettingsComponent::kDefaultBiomeBlend = 2;
int const SettingsComponent::kMaxBiomeBlend = 7;
int const SettingsComponent::kMinBiomeBlend = 0;

class GroupWater : public GroupComponent
{
public:
    std::function<void(float)> onWaterAbsorptionCoefficientChanged;
    std::function<void(bool)> onWaterTranslucentChanged;

public:
    GroupWater(Settings const& settings)
    {
        fWaterAbsorptionCoefficient = new Slider(Slider::LinearHorizontal, Slider::TextBoxBelow);
        fWaterAbsorptionCoefficient->setRange(SettingsComponent::kMinWaterAbsorptionCoefficient, SettingsComponent::kMaxWaterAbsorptionCoefficient);
        fWaterAbsorptionCoefficient->setValue(settings.fWaterOpticalDensity);
        fWaterAbsorptionCoefficient->onValueChange = [this]() {
            if (onWaterAbsorptionCoefficientChanged) {
                onWaterAbsorptionCoefficientChanged((float)fWaterAbsorptionCoefficient->getValue());
            }
        };
        addAndMakeVisible(fWaterAbsorptionCoefficient);
        
        fWaterAbsorptionCoefficientLabel = new Label();
        fWaterAbsorptionCoefficientLabel->setText("Optical Density", NotificationType::dontSendNotification);
        addAndMakeVisible(fWaterAbsorptionCoefficientLabel);
        
        fTranslucentWater = new ToggleButton("Translucent");
        fTranslucentWater->setToggleState(settings.fWaterTranslucent, NotificationType::dontSendNotification);
        addAndMakeVisible(fTranslucentWater);
        fTranslucentWater->onStateChange = [this]() {
            auto translucent = fTranslucentWater->getToggleState();
            if (onWaterTranslucentChanged) {
                onWaterTranslucentChanged(translucent);
            }
            fWaterAbsorptionCoefficient->setEnabled(translucent);
        };
        
        setText("Water");
        setSize(400, 140);
    }

    void resized() override {
        int const margin = 10;
        int const width = getWidth();
        int const rowHeight = 40;
        int const labelHeight = 20;
        int const rowMargin = 15;
        
        int y = margin;
        fTranslucentWater->setBounds(margin, y, width - 2 * margin, rowHeight);
        y += rowHeight;
        y += rowMargin;
        
        fWaterAbsorptionCoefficientLabel->setBounds(margin, y, width - 2 * margin, labelHeight);
        y += labelHeight;
        fWaterAbsorptionCoefficient->setBounds(margin, y, width - 2 * margin, rowHeight);
        y += rowHeight;
    }
    
private:
    ScopedPointer<Slider> fWaterAbsorptionCoefficient;
    ScopedPointer<Label> fWaterAbsorptionCoefficientLabel;
    
    ScopedPointer<ToggleButton> fTranslucentWater;
};

class GroupBiome : public GroupComponent
{
public:
    std::function<void(bool)> onEnableChanged;
    std::function<void(int)> onBiomeBlendChanged;
    
public:
    explicit GroupBiome(Settings const& settings)
    {
        setText("Biome");

        fEnableBiome = new ToggleButton("Enable");
        fEnableBiome->onStateChange = [this]() {
            bool const enable = fEnableBiome->getToggleState();
            if (onEnableChanged) {
                onEnableChanged(enable);
            }
            fBlend->setEnabled(enable);
        };
        fEnableBiome->setToggleState(settings.fBiomeEnabled, NotificationType::dontSendNotification);
        addAndMakeVisible(fEnableBiome);
        
        fBlendTitle = new Label();
        fBlendTitle->setText("Blend", NotificationType::dontSendNotification);
        addAndMakeVisible(fBlendTitle);
        
        fBlend = new Slider(Slider::LinearHorizontal, Slider::NoTextBox);
        fBlend->setRange(SettingsComponent::kMinBiomeBlend, SettingsComponent::kMaxBiomeBlend, 1);
        fBlend->setValue(settings.fBiomeBlend);
        fBlend->onValueChange = [this]() {
            int v = (int)fBlend->getValue();
            std::string text;
            if (v == 0) {
                text = "OFF";
            } else {
                std::ostringstream ss;
                ss << (v * 2 + 1) << "x" << (v * 2 + 1);
                text = ss.str();
            }
            fBlendLabel->setText(text, NotificationType::dontSendNotification);
            if (onBiomeBlendChanged) {
                onBiomeBlendChanged(v);
            }
        };
        addAndMakeVisible(fBlend);
        
        fBlendLabel = new Label();
        fBlendLabel->setText("3x3", NotificationType::dontSendNotification);
        addAndMakeVisible(fBlendLabel);
        
        setSize(400, 150);
    }
    
    void resized() override
    {
        int const margin = 10;
        int const width = getWidth();
        int const rowHeight = 40;
        
        int y = margin;
        fEnableBiome->setBounds(margin, margin, width - 2 * margin, rowHeight);
        y += rowHeight;
        y += margin;
        fBlendTitle->setBounds(margin, y, width - 2 * margin, 20);
        y += 20;
        fBlend->setBounds(margin, y, width - 2 * margin, rowHeight);
        y += rowHeight;
        fBlendLabel->setBounds(margin, y, width - 2 * margin, 20);
        y += 20;
    }

private:
    ScopedPointer<ToggleButton> fEnableBiome;
    ScopedPointer<Label> fBlendTitle;
    ScopedPointer<Slider> fBlend;
    ScopedPointer<Label> fBlendLabel;
};

SettingsComponent::SettingsComponent(Settings const& settings)
{
    ScopedPointer<GroupWater> water = new GroupWater(settings);
    water->onWaterTranslucentChanged = [this](bool translucent) {
        onWaterTranslucentChanged(translucent);
    };
    water->onWaterAbsorptionCoefficientChanged = [this](float v) {
        onWaterAbsorptionCoefficientChanged(v);
    };
    fGroupWater.reset(water.release());
    addAndMakeVisible(fGroupWater);
    
    ScopedPointer<GroupBiome> biome = new GroupBiome(settings);
    biome->onEnableChanged = [this](bool enable) {
        if (onBiomeEnableChanged) {
            onBiomeEnableChanged(enable);
        }
    };
    biome->onBiomeBlendChanged = [this](int v) {
        if (onBiomeBlendChanged) {
            onBiomeBlendChanged(v);
        }
    };
    fGroupBiome.reset(biome.release());
    addAndMakeVisible(fGroupBiome);
    
    setSize(250, 600);
}

void SettingsComponent::paint(Graphics &g)
{
    g.saveState();
    defer {
        g.restoreState();
    };

    int const width = getWidth();
    int const height = getHeight();
    int const margin = 10;
    
    g.setColour(Colours::white);
    g.drawText(String("Version: ") + String(ProjectInfo::versionString), 0, 0, width - margin, height - margin, Justification::bottomRight);
}

void SettingsComponent::resized()
{
    int const margin = 10;
    int const width = getWidth();
    int const rowMargin = 10;
    
    int y = margin;
    fGroupWater->setBounds(margin, y, width - 2 * margin, fGroupWater->getHeight());
    y += fGroupWater->getHeight();
    y += rowMargin;
    fGroupBiome->setBounds(margin, y, width - 2 * margin, fGroupBiome->getHeight());
    y += fGroupBiome->getHeight();
    y += rowMargin;
}
