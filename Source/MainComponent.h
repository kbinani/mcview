/*
  ==============================================================================

    This file was auto-generated!

  ==============================================================================
*/

#pragma once

#include "../JuceLibraryCode/JuceHeader.h"
#include "MapViewComponent.h"
#include "BrowserComponent.h"
#include "SettingsComponent.h"
#include "Dimension.h"
#include "Settings.h"
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
        fSettings = new Settings();
        fSettings->load();
        
        fMapViewComponent = new MapViewComponent();
        fMapViewComponent->setSize(600, 400);
        fMapViewComponent->onOpenButtonClicked = [this]() {
            setBrowserOpened(!fBrowserOpened);
        };
        fMapViewComponent->onSettingsButtonClicked = [this]() {
            setSettingsOpened(!fSettingsOpened);
        };
        
        fMapViewComponent->setWaterTranslucent(fSettings->fWaterTranslucent);
        fMapViewComponent->setWaterOpticalDensity(fSettings->fWaterOpticalDensity);
        fMapViewComponent->setBiomeEnable(fSettings->fBiomeEnabled);
        fMapViewComponent->setBiomeBlend(fSettings->fBiomeBlend);
        
        addAndMakeVisible(fMapViewComponent);

        fBrowser = new BrowserComponent();
        fBrowser->addDirectory(DefaultMinecraftSaveDirectory(), "Default");
        Array<File> directories = fSettings->directories();
        for (int i = 0; i < directories.size(); i++) {
            fBrowser->addDirectory(directories[i]);
        }
        fBrowser->onSelect = [this](File dir) {
            fMapViewComponent->setWorldDirectory(dir, Dimension::Overworld);
            getTopLevelComponent()->setName(dir.getFileName());
            setBrowserOpened(false);
        };
        fBrowser->onAdd = [this](File dir) {
            fSettings->addDirectory(dir);
            fSettings->save();
        };
        fBrowser->onRemove = [this](File dir) {
            fSettings->removeDirectory(dir);
            fSettings->save();
        };
        addAndMakeVisible(fBrowser);

        fSettingsComponent = new SettingsComponent(*fSettings);
        fSettingsComponent->onWaterOpticalDensityChanged = [this](float coeff) {
            fMapViewComponent->setWaterOpticalDensity(coeff);
            fSettings->fWaterOpticalDensity = coeff;
        };
        fSettingsComponent->onWaterTranslucentChanged = [this](bool translucent) {
            fMapViewComponent->setWaterTranslucent(translucent);
            fSettings->fWaterTranslucent = translucent;
        };
        fSettingsComponent->onBiomeEnableChanged = [this](bool enable) {
            fMapViewComponent->setBiomeEnable(enable);
            fSettings->fBiomeEnabled = enable;
        };
        fSettingsComponent->onBiomeBlendChanged = [this](int blend) {
            fMapViewComponent->setBiomeBlend(blend);
            fSettings->fBiomeBlend = blend;
        };
        addAndMakeVisible(fSettingsComponent);
        
		setSize(1280, 720);
	}

    ~MainComponent()
    {
        fSettings->save();
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
        if (!fMapViewComponent || !fBrowser || !fSettingsComponent) {
			return;
		}
        int const width = getWidth();
        int const height = getHeight();
        int const browserWidth = fBrowserOpened ? fBrowser->getWidth() : 0;
        int const settingsWidth = fSettingsOpened ? fSettingsComponent->getWidth() : 0;

        auto& animator = Desktop::getInstance().getAnimator();
        if (!animator.isAnimating(fBrowser)) {
            fBrowser->setBounds(fBrowserOpened ? 0 : -fBrowser->getWidth(), 0, fBrowser->getWidth(), height);
        }
        if (!animator.isAnimating(fMapViewComponent)) {
            fMapViewComponent->setBounds(browserWidth, 0, width - browserWidth - settingsWidth, height);
        }
        if (!animator.isAnimating(fSettingsComponent)) {
            fSettingsComponent->setBounds(width - settingsWidth, 0, fSettingsComponent->getWidth(), height);
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
        int const settingsWidth = fSettingsOpened ? fSettingsComponent->getWidth() : 0;

        if (opened) {
            int const w = fBrowser->getWidth();
            fBrowser->setBounds(-w, 0, w, height);
            Animate(fBrowser, 0, 0, w, height);
            fMapViewComponent->setBounds(0, 0, width - settingsWidth, height);
            Animate(fMapViewComponent, w, 0, width - w - settingsWidth, height);
        } else {
            int const w = fBrowser->getWidth();
            fBrowser->setBounds(0, 0, w, height);
            Animate(fBrowser, -w, 0, w, height);
            fMapViewComponent->setBounds(w, 0, width - w - settingsWidth, height);
            Animate(fMapViewComponent, 0, 0, width - settingsWidth, height);
        }
        fMapViewComponent->setBrowserOpened(opened);
    }
    
    void setSettingsOpened(bool opened) {
        if (fSettingsOpened == opened) {
            return;
        }
        fSettingsOpened = opened;
        
        int const width = getWidth();
        int const height = getHeight();

        int const browserWidth = fBrowserOpened ? fBrowser->getWidth() : 0;
        int const settingsWidth = fSettingsComponent->getWidth();
        
        fSettingsComponent->setVisible(fSettingsOpened);
        
        if (fSettingsOpened) {
            fSettingsComponent->setBounds(width, 0, settingsWidth, height);
            Animate(fSettingsComponent, width - settingsWidth, 0, settingsWidth, height);
            fMapViewComponent->setBounds(browserWidth, 0, width - browserWidth, height);
            Animate(fMapViewComponent, browserWidth, 0, width - browserWidth - settingsWidth, height);
        } else {
            fSettingsComponent->setBounds(width - settingsWidth, 0, settingsWidth, height);
            Animate(fSettingsComponent, width, 0, settingsWidth, height);
            fMapViewComponent->setBounds(browserWidth, 0, width - browserWidth - settingsWidth, height);
            Animate(fMapViewComponent, browserWidth, 0, width - browserWidth, height);
        }
    }
    
private:
    static void Animate(Component *comp, int x, int y, int width, int height) {
        auto& animator = Desktop::getInstance().getAnimator();
        animator.animateComponent(comp, { x, y, width, height }, 1.0f, kAnimationDuration, false, 0.0, 0.0);
    }
    
private:
    ScopedPointer<MapViewComponent> fMapViewComponent;
    ScopedPointer<BrowserComponent> fBrowser;
    bool fBrowserOpened;
    ScopedPointer<SettingsComponent> fSettingsComponent;
    bool fSettingsOpened;

    ScopedPointer<Settings> fSettings;
    
    static int constexpr kAnimationDuration = 300;
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MainComponent)
};
