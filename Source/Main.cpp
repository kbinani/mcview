/*
  ==============================================================================

    This file was auto-generated!

    It contains the basic startup code for a JUCE application.

  ==============================================================================
*/

#include <juce_gui_extra/juce_gui_extra.h>
#include "MainComponent.h"
#include "MapViewComponent.h"
#include "GraphicsHelper.h"
#include "LookAndFeel.h"
#include "LocalizationHelper.h"
#include "AboutComponent.h"

//==============================================================================
class mcviewApplication  : public juce::JUCEApplication
{
public:
    //==============================================================================
    mcviewApplication() {}

    const juce::String getApplicationName() override       { return JUCE_APPLICATION_NAME_STRING; }
    const juce::String getApplicationVersion() override    { return JUCE_APPLICATION_VERSION_STRING; }
    bool moreThanOneInstanceAllowed() override       { return true; }

    //==============================================================================
    void initialise (const juce::String&) override
    {
        using namespace juce;

        // This method is where you should put your application's initialisation code..

        LocalisedStrings::setCurrentMappings(LocalizationHelper::CurrentLocalisedStrings());

        fLookAndFeel.reset(new mcview::LookAndFeel());
        LookAndFeel::setDefaultLookAndFeel(fLookAndFeel.get());
        mainWindow.reset (new MainWindow (getApplicationName()));
    }

    void shutdown() override
    {
        // Add your application's shutdown code here..

        mainWindow = nullptr; // (deletes our window)
    }

    //==============================================================================
    void systemRequestedQuit() override
    {
        // This is called when the app is being asked to quit: you can ignore this
        // request and let the app carry on running, or call quit() to allow the app to close.
        quit();
    }
    
    void anotherInstanceStarted (const juce::String&) override
    {
        // When another instance of the app is launched while this one is running,
        // this method is invoked, and the commandLine parameter tells you what
        // the other instance's command-line arguments were.
    }

    //==============================================================================
    /*
        This class implements the desktop window that contains an instance of
        our MainComponent class.
    */
    class MainWindow    : public juce::DocumentWindow
    {
    public:
        MainWindow (juce::String name)  : juce::DocumentWindow (name,
            juce::Desktop::getInstance().getDefaultLookAndFeel().findColour (juce::ResizableWindow::backgroundColourId),
            juce::DocumentWindow::allButtons)
        {
            setUsingNativeTitleBar (true);
            setContentOwned (new MainComponent, true);

           #if JUCE_IOS || JUCE_ANDROID
            setFullScreen (true);
           #else
            setResizable (true, true);
            centreWithSize (getWidth(), getHeight());
           #endif

            setVisible (true);
        }

        void closeButtonPressed() override
        {
            // This is called when the user tries to close this window. Here, we'll just
            // ask the app to quit when this happens, but you can change this to do
            // whatever you need.
            if (!isPresentingAboutDialog()) {
                JUCEApplication::getInstance()->systemRequestedQuit();
            }
        }

        /* Note: Be careful if you override any DocumentWindow methods - the base
           class uses a lot of them, so by overriding you might break its functionality.
           It's best to do all your work in your content component instead, but if
           you really have to override any DocumentWindow methods, make sure your
           subclass also calls the superclass's method.
        */

    private:
        bool isPresentingAboutDialog() const
        {
            TopLevelWindow *top = TopLevelWindow::getActiveTopLevelWindow();
            if (!top) {
                return false;
            }
            DocumentWindow *doc = dynamic_cast<DocumentWindow *>(top);
            if (!doc) {
                return false;
            }
            Component *content = doc->getContentComponent();
            if (!content) {
                return false;
            }
            AboutComponent *about = dynamic_cast<AboutComponent *>(content);
            if (!about) {
                return false;
            }
            return true;
        }

    private:
        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MainWindow)
    };

private:
    std::unique_ptr<MainWindow> mainWindow;
    std::unique_ptr<mcview::LookAndFeel> fLookAndFeel;
};

//==============================================================================
// This macro generates the main() routine that launches the app.
START_JUCE_APPLICATION (mcviewApplication)
