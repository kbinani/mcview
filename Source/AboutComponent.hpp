#pragma once

namespace mcview {

class AboutComponent : public juce::Component {
public:
  AboutComponent() {
    using namespace juce;
    fLines = {
        CreateLabel(String("Version: ") + String::fromUTF8(JUCE_APPLICATION_VERSION_STRING)),
        CreateLabel("Copyright (C) 2019, 2022, 2023 kbinani"),
        CreateLabel(""),
        CreateLabel("Acknowledgement"),
        CreateLabel(""),

        CreateLabel("JUCE"),
        CreateLink("https://github.com/juce-framework/JUCE"),
        CreateLink("https://github.com/kbinani/JUCE"),
        CreateLabel(""),

        CreateLabel("Google Material Design Icon"),
        CreateLink("https://material.io/tools/icons"),
        CreateLabel(""),

        CreateLabel("colormap-shaders"),
        CreateLink("https://github.com/kbinani/colormap-shaders"),
        CreateLabel(""),

        CreateLabel("libminecraft-file"),
        CreateLink("https://github.com/kbinani/libminecraft-file"),
        CreateLabel(""),

        CreateLabel("Android SDK Sources"),
        CreateLink("https://github.com/AndroidSDKSources/android-sdk-sources-for-api-level-28"),
    };
    for (auto const &line : fLines) {
      addAndMakeVisible(*line);
    }
    fLogo = Drawable::createFromImageData(BinaryData::icon_large_png, BinaryData::icon_large_pngSize);
    setSize(400, 500);
  }

  void paint(juce::Graphics &g) override {
    using namespace juce;
    g.saveState();
    defer {
      g.restoreState();
    };

    int const margin = 10;
    int const width = getWidth();
    float y = margin;

    {
      int const logoHeight = 120;
      juce::Rectangle<float> logoArea(margin, y, width - 2 * margin, logoHeight);
      fLogo->drawWithin(g, logoArea, RectanglePlacement::centred, 1.0f);
      y += logoHeight + margin;
    }
    {
      int const titleHeight = 40;
      g.saveState();
      defer {
        g.restoreState();
      };
      g.setFont(titleHeight);
      g.setColour(Colours::white);
      GraphicsHelper::DrawText(g, JUCE_APPLICATION_NAME_STRING, margin, y, width - 2 * margin, titleHeight, Justification::centred);
      y += titleHeight + margin;
    }
    for (auto const &line : fLines) {
      line->setBounds(0, y, width, line->getHeight());
      y += line->getHeight();
    }
  }

private:
  static juce::Component *CreateLabel(juce::String t, int height = 14) {
    using namespace juce;
    Label *l = new Label();
    l->setText(t, NotificationType::dontSendNotification);
    l->setBounds(0, 0, 100, height);
    l->setColour(Label::textColourId, Colours::white);
    l->setJustificationType(Justification::centred);
    return l;
  }

  static juce::Component *CreateLink(juce::String t, int height = 16) {
    using namespace juce;
#if JUCE_WINDOWS
    return CreateLabel(t);
#else
    HyperlinkButton *b = new HyperlinkButton(t, URL(t));
    b->setBounds(0, 0, 100, height);
    b->setTooltip("");
    return b;
#endif
  }

private:
  std::unique_ptr<juce::Drawable> fLogo;
  juce::Array<std::unique_ptr<Component>> fLines;
};

} // namespace mcview
