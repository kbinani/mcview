#include "AboutComponent.h"
#include "GraphicsHelper.h"
#include "defer.h"

static Component* createLabel(String t, int height = 14)
{
    Label *l = new Label();
    l->setText(t, NotificationType::dontSendNotification);
    l->setBounds(0, 0, 100, height);
    l->setColour(Label::textColourId, Colours::white);
    l->setJustificationType(Justification::centred);
    return l;
}

static Component *createLink(String t, int height = 16)
{
#if JUCE_WINDOWS
    return createLabel(t);
#else
    HyperlinkButton *b = new HyperlinkButton(t, URL(t));
    b->setBounds(0, 0, 100, height);
    b->setTooltip("");
    return b;
#endif
}

AboutComponent::AboutComponent()
{
    fLines = {
        createLabel(String("Version: ") + String::fromUTF8(ProjectInfo::versionString)),
        createLabel("Copyright (C) 2019, 2022 kbinani"),
        createLabel(""),
        createLabel("Acknowledgement"),
        createLabel(""),
        
        createLabel("JUCE"),
        createLink("https://github.com/juce-framework/JUCE"),
        createLabel(""),
        
        createLabel("Google Material Design Icon"),
        createLink("https://material.io/tools/icons"),
        createLabel(""),
        
        createLabel("colormap-shaders"),
        createLink("https://github.com/kbinani/colormap-shaders"),
        createLabel(""),
        
        createLabel("libminecraft-file"),
        createLink("https://github.com/kbinani/libminecraft-file"),
        createLabel(""),
        
        createLabel("Android SDK Sources"),
        createLink("https://github.com/AndroidSDKSources/android-sdk-sources-for-api-level-28"),
    };
    for (auto const& line : fLines) {
        addAndMakeVisible(*line);
    }
    fLogo = Drawable::createFromImageData(BinaryData::icon_large_png, BinaryData::icon_large_pngSize);
    setSize(400, 500);
}

void AboutComponent::paint(Graphics &g)
{
    g.saveState();
    defer {
        g.restoreState();
    };
    
    int const margin = 10;
    int const width = getWidth();
    float y = margin;
    
    {
        int const logoHeight = 120;
        Rectangle<float> logoArea(margin, y, width - 2 * margin, logoHeight);
        fLogo->drawWithin(g, logoArea, RectanglePlacement::centred, 1.0f);
        y += logoHeight + margin;
    }
    {
        int const titleHeight = 40;
        g.saveState();
        defer {
            g.restoreState();
        };
        g.setFont(titleHeight);
        g.setColour(Colours::white);
        GraphicsHelper::DrawText(g, ProjectInfo::projectName, margin, y, width - 2 * margin, titleHeight, Justification::centred);
        y += titleHeight + margin;
    }
    for (auto const& line : fLines) {
        line->setBounds(0, y, width, line->getHeight());
        y += line->getHeight();
    }
}
