/*
  ==============================================================================

    This file was auto-generated!

  ==============================================================================
*/

#pragma once

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
class MainComponent   : public juce::Component, private juce::AsyncUpdater
{
public:
    //==============================================================================
    MainComponent()
        : fBrowserOpened(true)
        , fSettingsOpened(false)
    {
        using namespace juce;

        fSettings.reset(new Settings());
        fSettings->load();
        
        fMapViewComponent.reset(new MapViewComponent());
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
        fMapViewComponent->setShowPin(fSettings->fShowPin);
        
        addAndMakeVisible(fMapViewComponent.get());

        fBrowser.reset(new BrowserComponent());
#if !JUCE_MAC
        fBrowser->addDirectory(DefaultMinecraftSaveDirectory());
#endif
        Array<File> directories = fSettings->directories();
        for (int i = 0; i < directories.size(); i++) {
            fBrowser->addDirectory(directories[i]);
        }
#if JUCE_MAC
        if (directories.size() == 0) {
            triggerAsyncUpdate();
        }
#endif
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
        addAndMakeVisible(fBrowser.get());

        fSettingsComponent.reset(new SettingsComponent(*fSettings));
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
        fSettingsComponent->onShowPinChanged = [this](bool show) {
            fMapViewComponent->setShowPin(show);
            fSettings->fShowPin = show;
        };
        addAndMakeVisible(fSettingsComponent.get());
        
		setSize(1280, 720);
    }

    ~MainComponent()
    {
        fSettings->save();
    }
    
    void handleAsyncUpdate() override
    {
        fBrowser->browse();
    }

    //==============================================================================
    void paint (juce::Graphics& g) override
    {
        using namespace juce;
        // (Our component is opaque, so we must completely fill the background with a solid colour)
        g.fillAll (getLookAndFeel().findColour (ResizableWindow::backgroundColourId));
    }

    static juce::File DefaultMinecraftSaveDirectory()
    {
        using namespace juce;
#if JUCE_WINDOWS
        return File::getSpecialLocation(File::userApplicationDataDirectory).getChildFile(".minecraft").getChildFile("saves");
#else
        File library = File::getSpecialLocation(File::userHomeDirectory).getParentDirectory().getParentDirectory().getParentDirectory();
        return library.getChildFile("Application Support").getChildFile("minecraft").getChildFile("saves");
#endif
    }
    
    void resized() override
    {
        using namespace juce;

        if (!fMapViewComponent || !fBrowser || !fSettingsComponent) {
			return;
		}
        int const width = getWidth();
        int const height = getHeight();
        int const browserWidth = fBrowserOpened ? fBrowser->getWidth() : 0;
        int const settingsWidth = fSettingsOpened ? fSettingsComponent->getWidth() : 0;

        auto& animator = Desktop::getInstance().getAnimator();
        if (!animator.isAnimating(fBrowser.get())) {
            fBrowser->setBounds(fBrowserOpened ? 0 : -fBrowser->getWidth(), 0, fBrowser->getWidth(), height);
        }
        if (!animator.isAnimating(fMapViewComponent.get())) {
            fMapViewComponent->setBounds(browserWidth, 0, width - browserWidth - settingsWidth, height);
        }
        if (!animator.isAnimating(fSettingsComponent.get())) {
            fSettingsComponent->setBounds(width - settingsWidth, 0, fSettingsComponent->getWidth(), height);
        }
    }

    void childBoundsChanged (juce::Component *) override
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
            Animate(fBrowser.get(), 0, 0, w, height);
            fMapViewComponent->setBounds(0, 0, width - settingsWidth, height);
            Animate(fMapViewComponent.get(), w, 0, width - w - settingsWidth, height);
        } else {
            int const w = fBrowser->getWidth();
            fBrowser->setBounds(0, 0, w, height);
            Animate(fBrowser.get(), -w, 0, w, height);
            fMapViewComponent->setBounds(w, 0, width - w - settingsWidth, height);
            Animate(fMapViewComponent.get(), 0, 0, width - settingsWidth, height);
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
            Animate(fSettingsComponent.get(), width - settingsWidth, 0, settingsWidth, height);
            fMapViewComponent->setBounds(browserWidth, 0, width - browserWidth, height);
            Animate(fMapViewComponent.get(), browserWidth, 0, width - browserWidth - settingsWidth, height);
        } else {
            fSettingsComponent->setBounds(width - settingsWidth, 0, settingsWidth, height);
            Animate(fSettingsComponent.get(), width, 0, settingsWidth, height);
            fMapViewComponent->setBounds(browserWidth, 0, width - browserWidth - settingsWidth, height);
            Animate(fMapViewComponent.get(), browserWidth, 0, width - browserWidth, height);
        }
    }
    
private:
    static void Animate(juce::Component *comp, int x, int y, int width, int height) {
        using namespace juce;
        auto& animator = Desktop::getInstance().getAnimator();
        animator.animateComponent(comp, { x, y, width, height }, 1.0f, kAnimationDuration, false, 0.0, 0.0);
    }
    
private:
    std::unique_ptr<MapViewComponent> fMapViewComponent;
    std::unique_ptr<BrowserComponent> fBrowser;
    bool fBrowserOpened;
    std::unique_ptr<SettingsComponent> fSettingsComponent;
    bool fSettingsOpened;

    std::unique_ptr<Settings> fSettings;
    
    static int constexpr kAnimationDuration = 300;
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MainComponent)
};
