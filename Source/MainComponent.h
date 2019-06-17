/*
  ==============================================================================

    This file was auto-generated!

  ==============================================================================
*/

#pragma once

#include "../JuceLibraryCode/JuceHeader.h"
#include "MapViewComponent.h"
#include "Browser.h"
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
    {
        mapView = new MapViewComponent();
        mapView->setSize(600, 400);
        mapView->onOpenButtonClicked = [this]() {
            setBrowserOpened(!fBrowserOpened);
        };
        addAndMakeVisible(mapView);

        browser = new Browser();
        browser->addDirectory(DefaultMinecraftSaveDirectory());
        browser->onSelect = [this](File dir) {
            mapView->setRegionsDirectory(dir.getChildFile("region"));
            getTopLevelComponent()->setName(dir.getFileName());
            setBrowserOpened(false);
        };
        addAndMakeVisible(browser);

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
        if (!mapView || !browser) {
			return;
		}
		auto bounds = getBounds();
        const auto browserWidth = browser->getWidth();

        auto& animator = Desktop::getInstance().getAnimator();
        if (fBrowserOpened) {
            if (!animator.isAnimating(browser)) {
                browser->setBounds(0, 0, browserWidth, bounds.getHeight());
            }
            if (!animator.isAnimating(mapView)) {
                mapView->setBounds(browserWidth, 0, bounds.getWidth() - browserWidth, bounds.getHeight());
            }
        } else {
            if (!animator.isAnimating(mapView)) {
                mapView->setBounds(0, 0, bounds.getWidth(), bounds.getHeight());
            }
        }
    }

    void childBoundsChanged (Component *child) override
    {
        resized();
    }

    void setBrowserOpened(bool opened) {
        if (fBrowserOpened == opened) {
            return;
        }
        fBrowserOpened = opened;

        auto& animator = Desktop::getInstance().getAnimator();
        int const width = getWidth();
        int const height = getHeight();

        browser->setVisible(opened);
        int const duration = 300;

        if (opened) {
            int const w = browser->getWidth();
            browser->setBounds({ -w, 0, w, height });
            animator.animateComponent(browser, { 0, 0, w, height }, 1.0f, duration, false, 0.0, 0.0);
            mapView->setBounds({ 0, 0, width, height });
            animator.animateComponent(mapView, { w, 0, width - w, height }, 1.0f, duration, false, 0.0, 0.0);
        } else {
            int const w = browser->getWidth();
            browser->setBounds({ 0, 0, w, height });
            animator.animateComponent(browser, { -w, 0, w, height }, 1.0f, duration, false, 0.0, 0.0);
            mapView->setBounds({ w, 0, width - w, height });
            animator.animateComponent(mapView, { 0, 0, width, height }, 1.0f, duration, false, 0.0, 0.0);
        }
        mapView->setBrowserOpened(opened);
    }
    
private:
    ScopedPointer<MapViewComponent> mapView;
    ScopedPointer<Browser> browser;
    bool fBrowserOpened;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MainComponent)
};
