#include "SettingsComponent.h"
#include "defer.h"
#include "AboutComponent.h"

float const SettingsComponent::kDefaultWaterOpticalDensity = 0.02f;
float const SettingsComponent::kMaxWaterOpticalDensity = 0.04f;
float const SettingsComponent::kMinWaterOpticalDensity = 0.0f;

int const SettingsComponent::kDefaultBiomeBlend = 2;
int const SettingsComponent::kMaxBiomeBlend = 7;
int const SettingsComponent::kMinBiomeBlend = 0;

class GroupWater : public GroupComponent
{
public:
    std::function<void(float)> onWaterOpticalDensityChanged;
    std::function<void(bool)> onWaterTranslucentChanged;

public:
    GroupWater(Settings const& settings)
    {
        fWaterOpticalDensity.reset(new Slider(Slider::LinearHorizontal, Slider::TextBoxBelow));
        fWaterOpticalDensity->setRange(SettingsComponent::kMinWaterOpticalDensity, SettingsComponent::kMaxWaterOpticalDensity);
        fWaterOpticalDensity->setValue(settings.fWaterOpticalDensity);
        fWaterOpticalDensity->onValueChange = [this]() {
            if (onWaterOpticalDensityChanged) {
                onWaterOpticalDensityChanged((float)fWaterOpticalDensity->getValue());
            }
        };
        addAndMakeVisible(*fWaterOpticalDensity);
        
        fWaterOpticalDensityLabel.reset(new Label());
        fWaterOpticalDensityLabel->setText(TRANS("Optical Density"), NotificationType::dontSendNotification);
        addAndMakeVisible(*fWaterOpticalDensityLabel);
        
        fTranslucentWater.reset(new ToggleButton(TRANS("Translucent")));
        fTranslucentWater->setToggleState(settings.fWaterTranslucent, NotificationType::dontSendNotification);
        addAndMakeVisible(*fTranslucentWater);
        fTranslucentWater->onStateChange = [this]() {
            auto translucent = fTranslucentWater->getToggleState();
            if (onWaterTranslucentChanged) {
                onWaterTranslucentChanged(translucent);
            }
            fWaterOpticalDensity->setEnabled(translucent);
        };
        
        setText(TRANS("Water"));
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
        
        fWaterOpticalDensityLabel->setBounds(margin, y, width - 2 * margin, labelHeight);
        y += labelHeight;
        fWaterOpticalDensity->setBounds(margin, y, width - 2 * margin, rowHeight);
        y += rowHeight;
    }
    
private:
    std::unique_ptr<Slider> fWaterOpticalDensity;
    std::unique_ptr<Label> fWaterOpticalDensityLabel;
    
    std::unique_ptr<ToggleButton> fTranslucentWater;
};

class GroupBiome : public GroupComponent
{
public:
    std::function<void(bool)> onEnableChanged;
    std::function<void(int)> onBiomeBlendChanged;
    
public:
    explicit GroupBiome(Settings const& settings)
    {
        setText(TRANS("Biome"));

        fEnableBiome.reset(new ToggleButton(TRANS("Enable")));
        fEnableBiome->onStateChange = [this]() {
            bool const enable = fEnableBiome->getToggleState();
            if (onEnableChanged) {
                onEnableChanged(enable);
            }
            fBlend->setEnabled(enable);
        };
        fEnableBiome->setToggleState(settings.fBiomeEnabled, NotificationType::dontSendNotification);
        addAndMakeVisible(*fEnableBiome);
        
        fBlendTitle.reset(new Label());
        fBlendTitle->setText(TRANS("Blend"), NotificationType::dontSendNotification);
        addAndMakeVisible(*fBlendTitle);
        
        fBlend.reset(new Slider(Slider::LinearHorizontal, Slider::NoTextBox));
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
        addAndMakeVisible(*fBlend);
        
        fBlendLabel.reset(new Label());
        fBlendLabel->setText("3x3", NotificationType::dontSendNotification);
        addAndMakeVisible(*fBlendLabel);
        
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
    std::unique_ptr<ToggleButton> fEnableBiome;
    std::unique_ptr<Label> fBlendTitle;
    std::unique_ptr<Slider> fBlend;
    std::unique_ptr<Label> fBlendLabel;
};

class GroupPin : public GroupComponent {
public:
    std::function<void(bool)> onShowPinChanged;

    explicit GroupPin(Settings const& settings) {
        setText(TRANS("Pin"));
        fShowPin.reset(new ToggleButton(TRANS("Show")));
        fShowPin->setToggleState(settings.fShowPin, dontSendNotification);
        fShowPin->onStateChange = [this]() {
            if (onShowPinChanged) {
                onShowPinChanged(fShowPin->getToggleState());
            }
        };
        addAndMakeVisible(*fShowPin);
        setSize(400, 55);
    }

    void resized() override {
        int const margin = 10;
        int const width = getWidth();
        int const rowHeight = 40;

        int y = margin;
        fShowPin->setBounds(margin, margin, width - 2 * margin, rowHeight);
        y += rowHeight;
    }

private:
    std::unique_ptr<ToggleButton> fShowPin;
};

SettingsComponent::SettingsComponent(Settings const& settings)
{
    std::unique_ptr<GroupWater> water(new GroupWater(settings));
    water->onWaterTranslucentChanged = [this](bool translucent) {
        onWaterTranslucentChanged(translucent);
    };
    water->onWaterOpticalDensityChanged = [this](float v) {
        onWaterOpticalDensityChanged(v);
    };
    fGroupWater.reset(water.release());
    addAndMakeVisible(*fGroupWater);
    
    std::unique_ptr<GroupBiome> biome(new GroupBiome(settings));
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
    addAndMakeVisible(*fGroupBiome);

    std::unique_ptr<GroupPin> pin(new GroupPin(settings));
    pin->onShowPinChanged = [this](bool show) {
        if (onShowPinChanged) {
            onShowPinChanged(show);
        }
    };
    fGroupPin.reset(pin.release());
    addAndMakeVisible(*fGroupPin);

    fAboutButton.reset(new HyperlinkButton("About", URL()));
    fAboutButton->setJustificationType(Justification::centredRight);
    addAndMakeVisible(*fAboutButton);
    fAboutButton->onClick = [this]() {
        DialogWindow::LaunchOptions options;
        options.content.setOwned(new AboutComponent());
        options.dialogTitle = "About";
        options.useNativeTitleBar = true;
        options.escapeKeyTriggersCloseButton = true;
        options.resizable = false;
        options.dialogBackgroundColour = getLookAndFeel().findColour(ResizableWindow::backgroundColourId);
        options.runModal();
    };
    
    setSize(250, 600);
}

void SettingsComponent::paint(Graphics &g)
{
}

void SettingsComponent::resized()
{
    int const margin = 10;
    int const width = getWidth();
    int const height = getHeight();
    int const rowMargin = 10;
    
    {
        int y = margin;
        fGroupWater->setBounds(margin, y, width - 2 * margin, fGroupWater->getHeight());
        y += fGroupWater->getHeight();
        y += rowMargin;
        fGroupBiome->setBounds(margin, y, width - 2 * margin, fGroupBiome->getHeight());
        y += fGroupBiome->getHeight();
        y += rowMargin;
        fGroupPin->setBounds(margin, y, width - 2 * margin, fGroupPin->getHeight());
        y += fGroupPin->getHeight();
        y += rowMargin;
    }
    {
        int buttonHeight = 20;
        fAboutButton->setBounds(margin, height - margin - buttonHeight, width - 2 * margin, buttonHeight);
    }
}
