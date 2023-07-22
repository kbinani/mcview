#include <colormap/colormap.h>
#include <juce_gui_extra/juce_gui_extra.h>
#include <juce_opengl/juce_opengl.h>
#include <minecraft-file.hpp>

#include <filesystem>
#include <iostream>
#include <variant>

// clang-format off
#include "defer.hpp"

#include "BinaryData.h"
#include "GraphicsHelper.hpp"
#include "LocalizationHelper.hpp"
#include "LookAndFeel.hpp"
#include "Settings.hpp"
#include "File.hpp"
#include "LookAt.hpp"
#include "Dimension.hpp"
#include "Region.hpp"
#include "PNGWriter.hpp"
#include "Pin.hpp"
#include "WorldData.hpp"
#include "RegionTextureCache.hpp"
#include "OverScroller.hpp"
#include "TimerInstance.hpp"

#include "AboutComponent.hpp"
#include "SettingsComponent.hpp"
#include "DirectoryBrowserComponent.hpp"
#include "BrowserComponent.hpp"
#include "TextInputDialog.hpp"
#include "PinComponent.hpp"
#include "SavePNGProgressWindow.hpp"
#include "RegionToTexture.hpp"
#include "TexturePackJob.hpp"
#include "MapViewComponent.hpp"
#include "MainComponent.hpp"
#include "MainWindow.hpp"
// clang-format on

class Application : public juce::JUCEApplication {
public:
  Application() {}

  juce::String const getApplicationName() override {
    return JUCE_APPLICATION_NAME_STRING;
  }

  juce::String const getApplicationVersion() override {
    return JUCE_APPLICATION_VERSION_STRING;
  }

  bool moreThanOneInstanceAllowed() override {
    return true;
  }

  void initialise(juce::String const &) override {
    using namespace juce;

    LocalisedStrings::setCurrentMappings(LocalizationHelper::CurrentLocalisedStrings());

    fLookAndFeel.reset(new mcview::LookAndFeel());
    LookAndFeel::setDefaultLookAndFeel(fLookAndFeel.get());
    mainWindow.reset(new MainWindow(getApplicationName()));
  }

  void shutdown() override {
    mainWindow = nullptr;
  }

  void systemRequestedQuit() override {
    quit();
  }

  void anotherInstanceStarted(juce::String const &) override {
  }

private:
  std::unique_ptr<MainWindow> mainWindow;
  std::unique_ptr<mcview::LookAndFeel> fLookAndFeel;
};

START_JUCE_APPLICATION(Application)