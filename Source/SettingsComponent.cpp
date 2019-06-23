#include "SettingsComponent.h"

SettingsComponent::SettingsComponent()
{
    fWaterAbsorptionCoefficient = new Slider(Slider::LinearHorizontal, Slider::TextBoxBelow);
    fWaterAbsorptionCoefficient->setRange(0, 0.04);
    fWaterAbsorptionCoefficient->setValue(kDefaultWaterAbsorptionCoefficient);
    fWaterAbsorptionCoefficient->onValueChange = [this]() {
        onWaterAbsorptionCoefficientChanged((float)fWaterAbsorptionCoefficient->getValue());
    };
    addAndMakeVisible(fWaterAbsorptionCoefficient);
    
    fWaterAbsorptionCoefficientLabel = new Label();
    fWaterAbsorptionCoefficientLabel->setText("Water Absorption", NotificationType::dontSendNotification);
    addAndMakeVisible(fWaterAbsorptionCoefficientLabel);
    
    setSize(214, 600);
}

void SettingsComponent::paint(Graphics &g)
{
    
}

void SettingsComponent::resized()
{
    int const margin = 10;
    int const width = getWidth();
    int const rowHeight = 40;
    int const labelHeight = 20;
    
    int y = margin;
    fWaterAbsorptionCoefficientLabel->setBounds(margin, y, width - 2 * margin, labelHeight);
    y += labelHeight;
    fWaterAbsorptionCoefficient->setBounds(margin, y, width - 2 * margin, rowHeight);
    y += rowHeight;
}
