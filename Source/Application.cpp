#include <colormap/colormap.h>
#include <juce_gui_extra/juce_gui_extra.h>
#include <juce_opengl/juce_opengl.h>
#include <leveldb/env.h>
#include <minecraft-file.hpp>
#include <nlohmann/json.hpp>

#include <filesystem>
#include <iostream>
#include <latch>
#include <variant>

#include <je2be/integers.hpp>
#include <je2be/strings.hpp>

#include "db/_readonly-db.hpp"

// clang-format off
#include "defer.hpp"

#include "BinaryData.hpp"
#include "FontData.hpp"
#include "LocalizationHelper.hpp"
#include "NotoSansTypeface.hpp"
#include "LookAndFeel.hpp"
#include "PaletteType.hpp"
#include "LightingType.hpp"
#include "Edition.hpp"
#include "File.hpp"
#include "Directory.hpp"
#include "Settings.hpp"
#include "LookAt.hpp"
#include "Dimension.hpp"
#include "Region.hpp"
#include "PNGWriter.hpp"
#include "Pin.hpp"
#include "WorldData.hpp"
#include "RegionTextureCache.hpp"
#include "OverScroller.hpp"
#include "TimerInstance.hpp"
#include "ThreadPool.hpp"
#include "VisibleRegions.hpp"

#include "ImageButton.hpp"
#include "AboutComponent.hpp"
#include "SettingsComponent.hpp"
#include "GameDirectoryBrowserModel.hpp"
#include "GameDirectoryBrowser.hpp"
#include "CustomDirectoryBrowser.hpp"
#include "ConcertinaPanel.hpp"
#include "LeftPanelHeader.hpp"
#include "LeftPanel.hpp"
#include "TextInputDialog.hpp"
#include "PinComponent.hpp"
#include "SavePNGProgressWindow.hpp"
#include "Palette.hpp"
#include "RegionToTexture.hpp"
#include "TexturePackJob.hpp"
#include "JavaTexturePackJob.hpp"
#include "BedrockTexturePackJob.hpp"
#include "TexturePackThreadPool.hpp"
#include "JavaTexturePackThreadPool.hpp"
#include "BedrockTexturePackThreadPool.hpp"
#include "GLUniforms.hpp"
#include "GLVertex.hpp"
#include "GLAttributes.hpp"
#include "GLBuffer.hpp"
#include "PinEdit.hpp"
#include "WorldScanThread.hpp"
#include "JavaWorldScanThread.hpp"
#include "BedrockWorldScanThread.hpp"
#include "MapViewComponent.hpp"
#include "ColorMat.hpp"
#include "MainComponent.hpp"
#include "MainWindow.hpp"
#include "DirectoryCleanupThread.hpp"
// clang-format on

namespace mcview {

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
    return false;
  }

  void initialise(juce::String const &) override {
    using namespace juce;

    fCleanup.reset(new DirectoryCleanupThread);
    fCleanup->startThread();
    LocalisedStrings::setCurrentMappings(LocalizationHelper::CurrentLocalisedStrings());

    fLookAndFeel.reset(new mcview::LookAndFeel());
    LookAndFeel::setDefaultLookAndFeel(fLookAndFeel.get());
    mainWindow.reset(new MainWindow(getApplicationName()));
  }

  void shutdown() override {
    mainWindow = nullptr;
    if (fCleanup) {
      fCleanup->stopThread(-1);
    }
  }

  void systemRequestedQuit() override {
    quit();
  }

  void anotherInstanceStarted(juce::String const &) override {
  }

private:
  std::unique_ptr<MainWindow> mainWindow;
  std::unique_ptr<mcview::LookAndFeel> fLookAndFeel;
  std::unique_ptr<DirectoryCleanupThread> fCleanup;
};

} // namespace mcview

START_JUCE_APPLICATION(mcview::Application)
