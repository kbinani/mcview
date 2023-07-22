#pragma once

namespace mcview {

class AboutComponent : public juce::Component {
  class Content : public juce::Component {
  public:
    Content() {
      fLines = {
          CreateLabel(juce::String("Version: ") + juce::String::fromUTF8(JUCE_APPLICATION_VERSION_STRING)),
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
          CreateLabel(""),

          CreateLabel("Noto Sans"),
          CreateLink("https://fonts.google.com/noto/specimen/Noto+Sans"),
          CreateLabel(""),

          CreateLabel("Noto Sans JP"),
          CreateLink("https://fonts.google.com/noto/specimen/Noto+Sans+JP"),
          CreateLabel(""),

          CreateLabel("Noto Sans TC"),
          CreateLink("https://fonts.google.com/noto/specimen/Noto+Sans+TC"),
          CreateLabel(""),

          CreateLabel("Noto Sans SC"),
          CreateLink("https://fonts.google.com/noto/specimen/Noto+Sans+SC"),
          CreateLabel(""),
      };
      for (auto const &line : fLines) {
        addAndMakeVisible(*line);
      }
    }

    void resized() override {
      int const width = getWidth();
      int y = 0;
      for (auto const &line : fLines) {
        line->setBounds(5, y, width - 10, line->getHeight());
        y += line->getHeight();
      }
    }

    int getPreferredHeight() const {
      int h = 0;
      for (auto const &line : fLines) {
        h += line->getHeight();
      }
      return h;
    }

  private:
    juce::Array<std::unique_ptr<Component>> fLines;
  };

public:
  AboutComponent() {
    using namespace juce;
    fLogo = Drawable::createFromImageData(BinaryData::icon_large_png, BinaryData::icon_large_pngSize);
    fContent.reset(new Content);
    fViewport.reset(new juce::Viewport);
    fViewport->setViewedComponent(fContent.get(), false);
    fViewport->setScrollBarsShown(true, false);
    addAndMakeVisible(fViewport.get());
    setSize(400, 500);
  }

  void resized() override {
    auto bounds = getLocalBounds();
    bounds.removeFromTop(190);
    fContent->setSize(bounds.getWidth(), fContent->getPreferredHeight());
    fViewport->setBounds(bounds);
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
      g.drawText(JUCE_APPLICATION_NAME_STRING, margin, y, width - 2 * margin, titleHeight, Justification::centred);
      y += titleHeight + margin;
    }
  }

private:
  static juce::Component *CreateLabel(juce::String t, int height = 16) {
    using namespace juce;
    Label *l = new Label();
    l->setText(t, NotificationType::dontSendNotification);
    l->setBounds(0, 0, 100, height);
    l->setColour(Label::textColourId, Colours::white);
    l->setJustificationType(Justification::centred);
    l->setMinimumHorizontalScale(0);
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
  std::unique_ptr<Content> fContent;
  std::unique_ptr<juce::Viewport> fViewport;
};

} // namespace mcview
