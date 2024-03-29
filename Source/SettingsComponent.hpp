#pragma once

namespace mcview {

class SettingsComponent : public juce::Component {
  enum {
    kMargin = 10,
    kLabelHeight = 20,
    kRowHeight = 40,
    kRowMargin = 15,
  };

  class GroupWater : public juce::GroupComponent {
  public:
    std::function<void(float)> onWaterOpticalDensityChanged;
    std::function<void(bool)> onWaterTranslucentChanged;

  public:
    explicit GroupWater(Settings const &settings) {
      using namespace juce;
      fWaterOpticalDensity.reset(new Slider(Slider::LinearHorizontal, Slider::TextBoxBelow));
      fWaterOpticalDensity->setRange(Settings::kMinWaterOpticalDensity, Settings::kMaxWaterOpticalDensity);
      fWaterOpticalDensity->setValue(settings.fWaterOpticalDensity);
      fWaterOpticalDensity->setNumDecimalPlacesToDisplay(3);
      fWaterOpticalDensity->onValueChange = [this]() {
        if (onWaterOpticalDensityChanged) {
          onWaterOpticalDensityChanged((float)fWaterOpticalDensity->getValue());
        }
      };
      addAndMakeVisible(*fWaterOpticalDensity);

      fWaterOpticalDensityLabel.reset(new Label());
      fWaterOpticalDensityLabel->setText(TRANS("Optical Density"), NotificationType::dontSendNotification);
      addAndMakeVisible(*fWaterOpticalDensityLabel);

      fTranslucentWater.reset(new ToggleButton(TRANS("Translucent")));
      fTranslucentWater->setToggleState(settings.fWaterTranslucent, NotificationType::dontSendNotification);
      addAndMakeVisible(*fTranslucentWater);
      fTranslucentWater->onStateChange = [this]() {
        auto translucent = fTranslucentWater->getToggleState();
        if (onWaterTranslucentChanged) {
          onWaterTranslucentChanged(translucent);
        }
        fWaterOpticalDensity->setEnabled(translucent);
      };

      setText(TRANS("Water"));
      setSize(400, 140);
    }

    void resized() override {
      auto bounds = getLocalBounds();
      bounds.reduce(kMargin, kMargin);

      fTranslucentWater->setBounds(bounds.removeFromTop(kRowHeight));
      bounds.removeFromTop(kRowMargin);
      fWaterOpticalDensityLabel->setBounds(bounds.removeFromTop(kLabelHeight));
      fWaterOpticalDensity->setBounds(bounds.removeFromTop(kRowHeight));
    }

  private:
    std::unique_ptr<juce::Slider> fWaterOpticalDensity;
    std::unique_ptr<juce::Label> fWaterOpticalDensityLabel;

    std::unique_ptr<juce::ToggleButton> fTranslucentWater;
  };

  class GroupBiome : public juce::GroupComponent {
  public:
    std::function<void(bool)> onEnableChanged;
    std::function<void(int)> onBiomeBlendChanged;

  public:
    explicit GroupBiome(Settings const &settings) {
      using namespace juce;
      setText(TRANS("Biome"));

      fEnableBiome.reset(new ToggleButton(TRANS("Enable")));
      fEnableBiome->onStateChange = [this]() {
        bool const enable = fEnableBiome->getToggleState();
        if (onEnableChanged) {
          onEnableChanged(enable);
        }
        fBlend->setEnabled(enable);
      };
      fEnableBiome->setToggleState(settings.fBiomeEnabled, NotificationType::dontSendNotification);
      addAndMakeVisible(*fEnableBiome);

      fBlendTitle.reset(new Label());
      fBlendTitle->setText(TRANS("Blend"), NotificationType::dontSendNotification);
      addAndMakeVisible(*fBlendTitle);

      fBlend.reset(new Slider(Slider::LinearHorizontal, Slider::NoTextBox));
      fBlend->setRange(Settings::kMinBiomeBlend, Settings::kMaxBiomeBlend, 1);
      fBlend->setValue(settings.fBiomeBlend);
      fBlend->onValueChange = [this]() {
        int v = (int)fBlend->getValue();
        std::string text;
        if (v == 0) {
          text = "OFF";
        } else {
          std::ostringstream ss;
          ss << (v * 2 + 1) << "x" << (v * 2 + 1);
          text = ss.str();
        }
        fBlendLabel->setText(text, NotificationType::dontSendNotification);
        if (onBiomeBlendChanged) {
          onBiomeBlendChanged(v);
        }
      };
      addAndMakeVisible(*fBlend);

      fBlendLabel.reset(new Label());
      fBlendLabel->setText("3x3", NotificationType::dontSendNotification);
      addAndMakeVisible(*fBlendLabel);

      setSize(400, 150);
    }

    void resized() override {
      auto bounds = getLocalBounds();
      bounds.reduce(kMargin, kMargin);

      fEnableBiome->setBounds(bounds.removeFromTop(kRowHeight));
      bounds.removeFromTop(kMargin);
      fBlendTitle->setBounds(bounds.removeFromTop(kLabelHeight));
      fBlend->setBounds(bounds.removeFromTop(kRowHeight));
      fBlendLabel->setBounds(bounds.removeFromTop(kLabelHeight));
    }

  private:
    std::unique_ptr<juce::ToggleButton> fEnableBiome;
    std::unique_ptr<juce::Label> fBlendTitle;
    std::unique_ptr<juce::Slider> fBlend;
    std::unique_ptr<juce::Label> fBlendLabel;
  };

  class GroupOther : public juce::GroupComponent {
  public:
    std::function<void(PaletteType type)> onPaletteChanged;
    std::function<void(LightingType type)> onLightingChanged;
    std::function<void(bool)> onShowPinChanged;

    explicit GroupOther(Settings const &settings) {
      using namespace juce;
      setText(TRANS("Other"));

      fPaletteLabel.reset(new Label());
      fPaletteLabel->setText(TRANS("Block color palette"), dontSendNotification);
      addAndMakeVisible(*fPaletteLabel);

      fPalette.reset(new ComboBox);
      fPaletteItems = {
          {PaletteType::mcview, "mcview"},
          {PaletteType::java, "Java"},
          {PaletteType::bedrock, "Bedrock"},
      };
      for (auto const &it : fPaletteItems) {
        fPalette->addItem(it.second, static_cast<int>(it.first) + 1);
      }
      fPalette->setSelectedId(static_cast<int>(settings.fPaletteType) + 1, dontSendNotification);
      fPalette->onChange = [this]() {
        int id = fPalette->getSelectedId();
        if (auto found = fPaletteItems.find(static_cast<PaletteType>(id - 1)); found != fPaletteItems.end()) {
          onPaletteChanged(found->first);
        }
      };
      addAndMakeVisible(*fPalette);

      fLightingLabel.reset(new Label);
      fLightingLabel->setText(TRANS("Lighting type"), dontSendNotification);
      addAndMakeVisible(*fLightingLabel);

      fLighting.reset(new ComboBox);
      fLightingItems = {
          {LightingType::topLeft, TRANS("Top-left lighting (mcview original)")},
          {LightingType::top, TRANS("Top lighting (same as in-game map)")},
      };
      for (auto const &it : fLightingItems) {
        fLighting->addItem(it.second, static_cast<int>(it.first) + 1);
      }
      fLighting->setSelectedId(static_cast<int>(settings.fLightingType) + 1, dontSendNotification);
      fLighting->onChange = [this]() {
        int id = fLighting->getSelectedId();
        if (auto found = fLightingItems.find(static_cast<LightingType>(id - 1)); found != fLightingItems.end()) {
          onLightingChanged(found->first);
        }
      };
      addAndMakeVisible(*fLighting);

      fShowPin.reset(new ToggleButton(TRANS("Enable marker pin")));
      fShowPin->setToggleState(settings.fShowPin, juce::dontSendNotification);
      fShowPin->onStateChange = [this]() {
        if (onShowPinChanged) {
          onShowPinChanged(fShowPin->getToggleState());
        }
      };
      addAndMakeVisible(*fShowPin);
      setSize(400, 220);
    }

    void resized() override {
      auto bounds = getLocalBounds();
      bounds.reduce(kMargin, kMargin);

      bounds.removeFromTop(kMargin);
      fPaletteLabel->setBounds(bounds.removeFromTop(kLabelHeight));
      fPalette->setBounds(bounds.removeFromTop(kRowHeight));
      bounds.removeFromTop(kRowMargin);
      fLightingLabel->setBounds(bounds.removeFromTop(kLabelHeight));
      fLighting->setBounds(bounds.removeFromTop(kRowHeight));
      bounds.removeFromTop(kRowMargin);
      fShowPin->setBounds(bounds.removeFromTop(kRowHeight));
    }

  private:
    std::unique_ptr<juce::Label> fPaletteLabel;
    std::unique_ptr<juce::ComboBox> fPalette;
    std::map<PaletteType, juce::String> fPaletteItems;
    std::unique_ptr<juce::Label> fLightingLabel;
    std::unique_ptr<juce::ComboBox> fLighting;
    std::map<LightingType, juce::String> fLightingItems;
    std::unique_ptr<juce::ToggleButton> fShowPin;
  };

public:
  std::function<void(float)> onWaterOpticalDensityChanged;
  std::function<void(bool)> onWaterTranslucentChanged;
  std::function<void(bool)> onBiomeEnableChanged;
  std::function<void(int)> onBiomeBlendChanged;
  std::function<void(bool)> onShowPinChanged;
  std::function<void(PaletteType)> onPaletteChanged;
  std::function<void(LightingType type)> onLightingChanged;

  static constexpr int kDefaultWidth = 250;

public:
  explicit SettingsComponent(Settings const &settings) {
    using namespace juce;
    std::unique_ptr<GroupWater> water(new GroupWater(settings));
    water->onWaterTranslucentChanged = [this](bool translucent) {
      onWaterTranslucentChanged(translucent);
    };
    water->onWaterOpticalDensityChanged = [this](float v) {
      onWaterOpticalDensityChanged(v);
    };
    fGroupWater.reset(water.release());
    addAndMakeVisible(*fGroupWater);

    std::unique_ptr<GroupBiome> biome(new GroupBiome(settings));
    biome->onEnableChanged = [this](bool enable) {
      if (onBiomeEnableChanged) {
        onBiomeEnableChanged(enable);
      }
    };
    biome->onBiomeBlendChanged = [this](int v) {
      if (onBiomeBlendChanged) {
        onBiomeBlendChanged(v);
      }
    };
    fGroupBiome.reset(biome.release());
    addAndMakeVisible(*fGroupBiome);

    std::unique_ptr<GroupOther> other(new GroupOther(settings));
    other->onShowPinChanged = [this](bool show) {
      if (onShowPinChanged) {
        onShowPinChanged(show);
      }
    };
    other->onPaletteChanged = [this](PaletteType type) {
      if (onPaletteChanged) {
        onPaletteChanged(type);
      }
    };
    other->onLightingChanged = [this](LightingType type) {
      if (onLightingChanged) {
        onLightingChanged(type);
      }
    };
    fGroupOther.reset(other.release());
    addAndMakeVisible(*fGroupOther);

    fAboutButton.reset(new HyperlinkButton("About", URL()));
    fAboutButton->setJustificationType(Justification::centredRight);
    addAndMakeVisible(*fAboutButton);
    fAboutButton->onClick = [this]() {
      DialogWindow::LaunchOptions options;
      options.content.setOwned(new AboutComponent());
      options.dialogTitle = "About";
      options.useNativeTitleBar = true;
      options.escapeKeyTriggersCloseButton = true;
      options.resizable = false;
      options.dialogBackgroundColour = getLookAndFeel().findColour(ResizableWindow::backgroundColourId);
      // TODO:resource
      DialogWindow *dialog = options.launchAsync();
    };

    setSize(kDefaultWidth, 600);
  }

  void paint(juce::Graphics &g) override {
  }

  void resized() override {
    int const margin = 10;
    int const width = getWidth();
    int const height = getHeight();
    int const rowMargin = 10;

    {
      int y = margin;
      fGroupWater->setBounds(margin, y, width - 2 * margin, fGroupWater->getHeight());
      y += fGroupWater->getHeight();
      y += rowMargin;
      fGroupBiome->setBounds(margin, y, width - 2 * margin, fGroupBiome->getHeight());
      y += fGroupBiome->getHeight();
      y += rowMargin;
      fGroupOther->setBounds(margin, y, width - 2 * margin, fGroupOther->getHeight());
      y += fGroupOther->getHeight();
      y += rowMargin;
    }
    {
      int buttonHeight = 20;
      fAboutButton->setBounds(margin, height - margin - buttonHeight, width - 2 * margin, buttonHeight);
    }
  }

private:
  std::unique_ptr<juce::GroupComponent> fGroupWater;
  std::unique_ptr<juce::GroupComponent> fGroupBiome;
  std::unique_ptr<juce::GroupComponent> fGroupOther;
  std::unique_ptr<juce::HyperlinkButton> fAboutButton;

  JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(SettingsComponent)
};

} // namespace mcview
