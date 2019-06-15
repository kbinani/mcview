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
class MainComponent   : public Component, public MenuBarModel, public ApplicationCommandTarget
{
public:
    //==============================================================================
    MainComponent()
    {
        mapView = new MapViewComponent();
        mapView->setSize(600, 400);
        addAndMakeVisible(mapView);

        menuBar.reset(new MenuBarComponent(this));
		menuBar->setSize(600, kMenuHeight);
        addAndMakeVisible(menuBar);
#if JUCE_MAC
        MenuBarModel::setMacMainMenu(this);
#endif

        browser = new Browser();
        browser->addDirectory(DefaultMinecraftSaveDirectory());
        browser->onSelect = [this](File dir) {
            mapView->setRegionsDirectory(dir.getChildFile("region"));
        };
        browser->setSize(300, 400);
        addAndMakeVisible(browser);

        setApplicationCommandManagerToWatch (&commandManager);
        commandManager.registerAllCommandsForTarget(this);

		setSize(600, 400);
	}

    ~MainComponent()
    {
#if JUCE_MAC
        MenuBarModel::setMacMainMenu (nullptr);
#endif
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
        // This is called when the MainComponent is resized.
        // If you add any child components, this is where you should
        // update their positions.
        if (!menuBar || !mapView || !browser) {
			return;
		}
		auto bounds = getBounds();
#if JUCE_MAC
		menuBar->setBounds(0, 0, bounds.getWidth(), 0);
#else
		menuBar->setBounds(0, 0, bounds.getWidth(), kMenuHeight);
#endif
		const auto menuHeight = menuBar->getHeight();
        const auto browserWidth = browser->getWidth();
    
        browser->setBounds(0, menuHeight, browserWidth, bounds.getHeight() - menuHeight);
        mapView->setBounds(browserWidth, menuHeight, bounds.getWidth() - browserWidth, bounds.getHeight() - menuHeight);
    }

    void childBoundsChanged (Component *child) override
    {
        resized();
    }
    
    enum CommandIDs
    {
        fileSelectRootDir = 1,
    };

    StringArray getMenuBarNames() override {
        return { "File" };
    }

    PopupMenu getMenuForIndex (int topLevelMenuIndex,
                                       const String& menuName) override {
        PopupMenu menu;

        switch (topLevelMenuIndex) {
            case 0:
                menu.addCommandItem(&commandManager, CommandIDs::fileSelectRootDir);
            default:
                break;
        }
        return menu;
        
    }

    void menuItemSelected (int menuItemID,
                                   int topLevelMenuIndex) override {
        switch (menuItemID) {
            case fileSelectRootDir:
                break;
            default:
                break;
        }
    }

    ApplicationCommandTarget* getNextCommandTarget() override {
        return nullptr;
    }

    void getAllCommands (Array<CommandID>& c) override {
        Array<CommandID> commands = { CommandIDs::fileSelectRootDir };
        c.addArray(commands);
    }
    
    void getCommandInfo (CommandID commandID, ApplicationCommandInfo& result) override {
        switch (commandID) {
            case fileSelectRootDir:
                result.setInfo("Select world", "", "Menu", 0);
                break;
            default:
                break;
        }
    }

    bool perform (const InvocationInfo& info) override {
        switch (info.commandID) {
            case fileSelectRootDir: {
                FileChooser dialog("Select world");
                if (!dialog.browseForDirectory()) {
                    return true;
                }
                File dir = dialog.getResult();
                if (dir.getChildFile("region").isDirectory()) {
                    dir = dir.getChildFile("region");
                }
                mapView->setRegionsDirectory(dir);
                return true;
            }
            default:
                return false;
        }
    }
    
private:
    ScopedPointer<MapViewComponent> mapView;
    ScopedPointer<MenuBarComponent> menuBar;
    ScopedPointer<Browser> browser;

    ApplicationCommandManager commandManager;

	static int constexpr kMenuHeight = 24;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MainComponent)
};
