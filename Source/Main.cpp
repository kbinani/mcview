/*
  ==============================================================================

    This file was auto-generated!

    It contains the basic startup code for a JUCE application.

  ==============================================================================
*/

#include <juce_gui_extra/juce_gui_extra.h>

#include "AboutComponent.hpp"
#include "GraphicsHelper.hpp"
#include "LocalizationHelper.hpp"
#include "LookAndFeel.hpp"
#include "MainComponent.hpp"
#include "MainWindow.hpp"
#include "MapViewComponent.hpp"

//==============================================================================
class mcviewApplication : public juce::JUCEApplication {
public:
  //==============================================================================
  mcviewApplication() {}

  const juce::String getApplicationName() override { return JUCE_APPLICATION_NAME_STRING; }
  const juce::String getApplicationVersion() override { return JUCE_APPLICATION_VERSION_STRING; }
  bool moreThanOneInstanceAllowed() override { return true; }

  //==============================================================================
  void initialise(const juce::String &) override {
    using namespace juce;

    // This method is where you should put your application's initialisation code..

    LocalisedStrings::setCurrentMappings(LocalizationHelper::CurrentLocalisedStrings());

    fLookAndFeel.reset(new mcview::LookAndFeel());
    LookAndFeel::setDefaultLookAndFeel(fLookAndFeel.get());
    mainWindow.reset(new MainWindow(getApplicationName()));
  }

  void shutdown() override {
    // Add your application's shutdown code here..

    mainWindow = nullptr; // (deletes our window)
  }

  //==============================================================================
  void systemRequestedQuit() override {
    // This is called when the app is being asked to quit: you can ignore this
    // request and let the app carry on running, or call quit() to allow the app to close.
    quit();
  }

  void anotherInstanceStarted(const juce::String &) override {
    // When another instance of the app is launched while this one is running,
    // this method is invoked, and the commandLine parameter tells you what
    // the other instance's command-line arguments were.
  }

private:
  std::unique_ptr<MainWindow> mainWindow;
  std::unique_ptr<mcview::LookAndFeel> fLookAndFeel;
};

//==============================================================================
// This macro generates the main() routine that launches the app.
START_JUCE_APPLICATION(mcviewApplication)
