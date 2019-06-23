/*
  ==============================================================================

    This file was auto-generated!

  ==============================================================================
*/

#pragma once

#include "../JuceLibraryCode/JuceHeader.h"
#include "MapViewComponent.h"
#include "Browser.h"
#include "SettingsComponent.h"
#include <iostream>

//==============================================================================
/*
    This component lives inside our window, and this is where you should put all
    your controls and content.
*/
class MainComponent   : public Component
{
public:
    //==============================================================================
    MainComponent()
        : fBrowserOpened(true)
        , fSettingsOpened(false)
    {
        fMapView = new MapViewComponent();
        fMapView->setSize(600, 400);
        fMapView->onOpenButtonClicked = [this]() {
            setBrowserOpened(!fBrowserOpened);
        };
        fMapView->onSettingsButtonClicked = [this]() {
            setSettingsOpened(!fSettingsOpened);
        };
        addAndMakeVisible(fMapView);

        fBrowser = new Browser();
        fBrowser->addDirectory(DefaultMinecraftSaveDirectory(), "Default");
        fBrowser->onSelect = [this](File dir) {
            fMapView->setRegionsDirectory(dir.getChildFile("region"));
            getTopLevelComponent()->setName(dir.getFileName());
            setBrowserOpened(false);
        };
        addAndMakeVisible(fBrowser);

        fSettings = new SettingsComponent();
        fSettings->onWaterAbsorptionCoefficientChanged = [this](float coeff) {
            fMapView->setWaterAbsorptionCoefficient(coeff);
        };
        fSettings->onWaterTranslucentChanged = [this](bool translucent) {
            fMapView->setWaterTranslucent(translucent);
        };
        fSettings->onBiomeEnableChanged = [this](bool enable) {
            fMapView->setBiomeEnable(enable);
        };
        fSettings->onBiomeBlendChanged = [this](int blend) {
            fMapView->setBiomeBlend(blend);
        };
        addAndMakeVisible(fSettings);
        
		setSize(1280, 720);
	}

    ~MainComponent()
    {
    }

    //==============================================================================
    void paint (Graphics& g) override
    {
        // (Our component is opaque, so we must completely fill the background with a solid colour)
        g.fillAll (getLookAndFeel().findColour (ResizableWindow::backgroundColourId));
    }

    static File DefaultMinecraftSaveDirectory()
    {
#if JUCE_WINDOWS
        return File::getSpecialLocation(File::userApplicationDataDirectory).getChildFile(".minecraft").getChildFile("saves");
#else
        return File::getSpecialLocation(File::userApplicationDataDirectory).getChildFile("Application Support").getChildFile("minecraft").getChildFile("saves");
#endif
    }
    
    void resized() override
    {
        if (!fMapView || !fBrowser || !fSettings) {
			return;
		}
        int const width = getWidth();
        int const height = getHeight();
        int const browserWidth = fBrowserOpened ? fBrowser->getWidth() : 0;
        int const settingsWidth = fSettingsOpened ? fSettings->getWidth() : 0;

        auto& animator = Desktop::getInstance().getAnimator();
        if (!animator.isAnimating(fBrowser)) {
            fBrowser->setBounds(fBrowserOpened ? 0 : -fBrowser->getWidth(), 0, fBrowser->getWidth(), height);
        }
        if (!animator.isAnimating(fMapView)) {
            fMapView->setBounds(browserWidth, 0, width - browserWidth - settingsWidth, height);
        }
        if (!animator.isAnimating(fSettings)) {
            fSettings->setBounds(width - settingsWidth, 0, fSettings->getWidth(), height);
        }
    }

    void childBoundsChanged (Component *) override
    {
        resized();
    }

    void setBrowserOpened(bool opened) {
        if (fBrowserOpened == opened) {
            return;
        }
        fBrowserOpened = opened;

        int const width = getWidth();
        int const height = getHeight();

        fBrowser->setVisible(opened);
        int const settingsWidth = fSettingsOpened ? fSettings->getWidth() : 0;

        if (opened) {
            int const w = fBrowser->getWidth();
            fBrowser->setBounds(-w, 0, w, height);
            Animate(fBrowser, 0, 0, w, height);
            fMapView->setBounds(0, 0, width - settingsWidth, height);
            Animate(fMapView, w, 0, width - w - settingsWidth, height);
        } else {
            int const w = fBrowser->getWidth();
            fBrowser->setBounds(0, 0, w, height);
            Animate(fBrowser, -w, 0, w, height);
            fMapView->setBounds(w, 0, width - w - settingsWidth, height);
            Animate(fMapView, 0, 0, width - settingsWidth, height);
        }
        fMapView->setBrowserOpened(opened);
    }
    
    void setSettingsOpened(bool opened) {
        if (fSettingsOpened == opened) {
            return;
        }
        fSettingsOpened = opened;
        
        int const width = getWidth();
        int const height = getHeight();

        int const browserWidth = fBrowserOpened ? fBrowser->getWidth() : 0;
        int const settingsWidth = fSettings->getWidth();
        
        fSettings->setVisible(fSettingsOpened);
        
        if (fSettingsOpened) {
            fSettings->setBounds(width, 0, settingsWidth, height);
            Animate(fSettings, width - settingsWidth, 0, settingsWidth, height);
            fMapView->setBounds(browserWidth, 0, width - browserWidth, height);
            Animate(fMapView, browserWidth, 0, width - browserWidth - settingsWidth, height);
        } else {
            fSettings->setBounds(width - settingsWidth, 0, settingsWidth, height);
            Animate(fSettings, width, 0, settingsWidth, height);
            fMapView->setBounds(browserWidth, 0, width - browserWidth - settingsWidth, height);
            Animate(fMapView, browserWidth, 0, width - browserWidth, height);
        }
    }
    
private:
    static void Animate(Component *comp, int x, int y, int width, int height) {
        auto& animator = Desktop::getInstance().getAnimator();
        animator.animateComponent(comp, { x, y, width, height }, 1.0f, kAnimationDuration, false, 0.0, 0.0);
    }
    
private:
    ScopedPointer<MapViewComponent> fMapView;
    ScopedPointer<Browser> fBrowser;
    bool fBrowserOpened;
    ScopedPointer<SettingsComponent> fSettings;
    bool fSettingsOpened;

    static int constexpr kAnimationDuration = 300;
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MainComponent)
};
