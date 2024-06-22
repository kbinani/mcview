#pragma once

namespace mcview {

class MainComponent : public juce::Component,
                      private juce::AsyncUpdater,
                      public MapViewComponent::Delegate,
                      public juce::Timer,
                      public juce::FileDragAndDropTarget {
public:
  struct Delegate {
    virtual ~Delegate() = default;
    virtual void mainComponentDidClose() = 0;
  };

  explicit MainComponent(Delegate *delegate)
      : fBrowserOpened(true), fSettingsOpened(false), fDelegate(delegate) {
    using namespace juce;

    fSettings.reset(new Settings());
    fSettings->load();

    fMapViewComponent.reset(new MapViewComponent(this));
    fMapViewComponent->setSize(600, 400);
    fMapViewComponent->setWaterTranslucent(fSettings->fWaterTranslucent);
    fMapViewComponent->setWaterOpticalDensity(fSettings->fWaterOpticalDensity);
    fMapViewComponent->setBiomeEnable(fSettings->fBiomeEnabled);
    fMapViewComponent->setBiomeBlend(fSettings->fBiomeBlend);
    fMapViewComponent->setPaletteType(fSettings->fPaletteType);
    fMapViewComponent->setLightingType(fSettings->fLightingType);
    fMapViewComponent->setShowPin(fSettings->fShowPin);

    addAndMakeVisible(fMapViewComponent.get());

    fBrowser.reset(new LeftPanel());
#if !JUCE_MAC
    Directory java;
    java.fDirectory = DefaultJavaSaveDirectory();
    java.fEdition = Edition::Java;
    fBrowser->addDirectory(java);

    Directory bedrock;
    bedrock.fDirectory = DefaultBedrockSaveDirectory();
    bedrock.fEdition = Edition::Bedrock;
    fBrowser->addDirectory(bedrock);
#endif
    Array<Directory> directories = fSettings->directories();
    for (int i = 0; i < directories.size(); i++) {
      fBrowser->addDirectory(directories[i]);
    }
#if JUCE_MAC
    if (directories.size() == 0) {
      triggerAsyncUpdate();
    }
#endif
    fBrowser->onSelect = [this](Directory d) {
      onSelect(d);
    };
    fBrowser->onAdd = [this](Directory d) {
      fSettings->addDirectory(d);
      fSettings->save();
    };
    fBrowser->onRemove = [this](Directory d) {
      fSettings->removeDirectory(d);
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
    fSettingsComponent->onPaletteChanged = [this](PaletteType palette) {
      fMapViewComponent->setPaletteType(palette);
      fSettings->fPaletteType = palette;
    };
    fSettingsComponent->onLightingChanged = [this](LightingType type) {
      fMapViewComponent->setLightingType(type);
      fSettings->fLightingType = type;
    };
    addAndMakeVisible(fSettingsComponent.get());

    fConcealer.reset(new ColorMat(juce::Colours::black.withAlpha(0.5f)));
    addChildComponent(*fConcealer);

    setSize(1280, 720);
  }

  ~MainComponent() {
    fSettings->save();
  }

  void handleAsyncUpdate() override {
    fBrowser->browseDefaultJavaSaves();
  }

  void paint(juce::Graphics &g) override {
    g.fillAll(getLookAndFeel().findColour(juce::ResizableWindow::backgroundColourId));
  }

  void resized() override {
    using namespace juce;

    if (!fMapViewComponent || !fBrowser || !fSettingsComponent) {
      return;
    }
    int const width = getWidth();
    int const height = getHeight();
    int browserWidth, mapWidth, settingsWidth;
    int actualBrowserWidth, actualSettingsWidth;
    decideWidths(browserWidth, mapWidth, settingsWidth, actualBrowserWidth, actualSettingsWidth);

    auto &animator = Desktop::getInstance().getAnimator();
    if (!animator.isAnimating(fBrowser.get())) {
      fBrowser->setBounds(browserWidth - actualBrowserWidth, 0, actualBrowserWidth, height);
    }
    if (!animator.isAnimating(fMapViewComponent.get())) {
      fMapViewComponent->setBounds(browserWidth, 0, mapWidth, height);
    }
    if (!animator.isAnimating(fSettingsComponent.get())) {
      fSettingsComponent->setBounds(browserWidth + mapWidth, 0, actualSettingsWidth, height);
    }
    fConcealer->setBounds(0, 0, width, height);
  }

  void startClosing() {
    fConcealer->setVisible(true);
    fConcealer->setAlwaysOnTop(true);
    fConcealer->setBounds(getLocalBounds());
    fMapViewComponent->startClosing();
    repaint();
  }

  bool isClosing() const {
    return fConcealer->isVisible();
  }

  void childBoundsChanged(juce::Component *) override {
    resized();
  }

  void timerCallback() override {
    stopTimer();
    auto &animator = juce::Desktop::getInstance().getAnimator();
    animator.cancelAllAnimations(true);
    resized();
  }

  void setBrowserOpened(bool opened) {
    if (fBrowserOpened == opened) {
      return;
    }
    fBrowserOpened = opened;
    fBrowser->setVisible(opened);
    fMapViewComponent->setBrowserOpened(opened);

    startAnimations();
  }

  void setSettingsOpened(bool opened) {
    if (fSettingsOpened == opened) {
      return;
    }
    fSettingsOpened = opened;
    fSettingsComponent->setVisible(fSettingsOpened);

    startAnimations();
  }

  void mainViewComponentOpenButtonClicked() override {
    setBrowserOpened(!fBrowserOpened);
  }

  void mainViewComponentSettingsButtonClicked() override {
    setSettingsOpened(!fSettingsOpened);
  }

  void mainViewComponentClosed() override {
    fDelegate->mainComponentDidClose();
  }

  bool isInterestedInFileDrag(juce::StringArray const &files) override {
    for (auto const &file : files) {
      if (auto d = Directory::Make(file); d) {
        return true;
      }
    }
    return false;
  }

  void filesDropped(juce::StringArray const &files, int x, int y) override {
    std::optional<Directory> open;
    int num = 0;
    for (auto const &file : files) {
      if (auto d = Directory::Make(file); d) {
        fBrowser->addDirectory(*d);
        if (!open) {
          open = *d;
          num++;
        }
      }
    }
    if (num == 1 && open) {
      onSelect(*open);
    }
  }

private:
  void onSelect(Directory d) {
    fMapViewComponent->setWorldDirectory(d.fDirectory, Dimension::Overworld, d.fEdition);
    getTopLevelComponent()->setName(d.fDirectory.getFileName() + " (" + d.fLevelName + ")");
    setBrowserOpened(false);
  }

  void decideWidths(int &browserWidth, int &mapWidth, int &settingsWidth, int &actualBrowserWidth, int &actualSettingsWidth) {
    int const width = getWidth();
    browserWidth = fBrowserOpened ? fBrowser->getWidth() : 0;
    settingsWidth = fSettingsOpened ? fSettingsComponent->getWidth() : 0;
    mapWidth = std::max(MapViewComponent::kMinimumWidth, width - browserWidth - settingsWidth);
    int widthOverflow = mapWidth + browserWidth + settingsWidth - width;
    if (widthOverflow > 0) {
      browserWidth = std::max(LeftPanel::kMinimumWidth, browserWidth - widthOverflow / 2);
      settingsWidth = std::max(0, width - mapWidth - browserWidth);
    }
    if (fBrowserOpened) {
      actualBrowserWidth = browserWidth;
    } else {
      actualBrowserWidth = fBrowser->getWidth();
    }
    actualSettingsWidth = SettingsComponent::kDefaultWidth;
  }

  void startAnimations() {
    int const width = getWidth();
    int const height = getHeight();
    int browserWidth, mapWidth, settingsWidth;
    int actualBrowserWidth, actualSettingsWidth;
    decideWidths(browserWidth, mapWidth, settingsWidth, actualBrowserWidth, actualSettingsWidth);

    Animate(fBrowser.get(), browserWidth - actualBrowserWidth, 0, actualBrowserWidth, height);
    Animate(fMapViewComponent.get(), browserWidth, 0, mapWidth, height);
    Animate(fSettingsComponent.get(), width - settingsWidth, 0, actualSettingsWidth, height);
    startTimer(kAnimationDuration);
  }

  static void Animate(juce::Component *comp, int x, int y, int width, int height) {
    auto &animator = juce::Desktop::getInstance().getAnimator();
    animator.animateComponent(comp, {x, y, width, height}, 1.0f, kAnimationDuration, false, 0.0, 0.0);
  }

private:
  std::unique_ptr<MapViewComponent> fMapViewComponent;
  std::unique_ptr<LeftPanel> fBrowser;
  bool fBrowserOpened;
  std::unique_ptr<SettingsComponent> fSettingsComponent;
  bool fSettingsOpened;
  std::unique_ptr<Settings> fSettings;
  std::unique_ptr<ColorMat> fConcealer;
  Delegate *const fDelegate;

  static int constexpr kAnimationDuration = 300;

  JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(MainComponent)
};

} // namespace mcview
