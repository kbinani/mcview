/*
  ==============================================================================

    This file was auto-generated!

    It contains the basic startup code for a JUCE application.

  ==============================================================================
*/

#include "../JuceLibraryCode/JuceHeader.h"
#include "MainComponent.h"
#include "MapViewComponent.h"
#include "GraphicsHelper.h"

class LAF : public LookAndFeel_V4
{
public:
    void drawConcertinaPanelHeader (Graphics &g, const Rectangle< int > &area, bool isMouseOver, bool isMouseDown, ConcertinaPanel &panel, Component &component) override
    {
        LookAndFeel_V4::drawConcertinaPanelHeader(g, area, isMouseOver, isMouseDown, panel, component);
        auto const name = component.getName();
        g.setColour(Colours::white);
        int const margin = 10;
        GraphicsHelper::DrawFittedText(g, name, margin, 0, area.getWidth() - 2 * margin, area.getHeight(), Justification::centredLeft, 1);
    }
};

static LocalisedStrings* LoadLocalisedStrings(char const* data, int size)
{
    std::vector<char> d(size + 1);
    std::copy_n(data, size, d.begin());
    String t = String::fromUTF8(d.data());
    return new LocalisedStrings(t, false);
}

//==============================================================================
class mcviewApplication  : public JUCEApplication
{
public:
    //==============================================================================
    mcviewApplication() {}

    const String getApplicationName() override       { return ProjectInfo::projectName; }
    const String getApplicationVersion() override    { return ProjectInfo::versionString; }
    bool moreThanOneInstanceAllowed() override       { return true; }

    //==============================================================================
    void initialise (const String&) override
    {
        // This method is where you should put your application's initialisation code..

        LocalisedStrings *jp = LoadLocalisedStrings(BinaryData::japanese_lang, BinaryData::japanese_langSize);
        LocalisedStrings::setCurrentMappings(jp);

        fLookAndFeel = new LAF();
        LookAndFeel::setDefaultLookAndFeel(fLookAndFeel);
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

    void anotherInstanceStarted (const String&) override
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
    class MainWindow    : public DocumentWindow
    {
    public:
        MainWindow (String name)  : DocumentWindow (name,
                                                    Desktop::getInstance().getDefaultLookAndFeel()
                                                                          .findColour (ResizableWindow::backgroundColourId),
                                                    DocumentWindow::allButtons)
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
            JUCEApplication::getInstance()->systemRequestedQuit();
        }

        /* Note: Be careful if you override any DocumentWindow methods - the base
           class uses a lot of them, so by overriding you might break its functionality.
           It's best to do all your work in your content component instead, but if
           you really have to override any DocumentWindow methods, make sure your
           subclass also calls the superclass's method.
        */

    private:
        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MainWindow)
    };

private:
    std::unique_ptr<MainWindow> mainWindow;
    ScopedPointer<LAF> fLookAndFeel;
};

//==============================================================================
// This macro generates the main() routine that launches the app.
START_JUCE_APPLICATION (mcviewApplication)
