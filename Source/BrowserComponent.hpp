#pragma once

namespace mcview {

class BrowserComponent : public juce::Component, private juce::Timer {
  class Header : public juce::Component {
  public:
    Header(juce::ConcertinaPanel *parent, juce::File file, juce::String const &title, bool removable)
        : fMouseDown(false), fMouseOver(false), fParent(parent), fTitle(title), fFile(file) {
      if (removable) {
        fButtonIcon.reset(juce::Drawable::createFromImageData(BinaryData::baseline_remove_white_18dp_png,
                                                              BinaryData::baseline_remove_white_18dp_pngSize)
                              .release());
        fButton.reset(new juce::DrawableButton("", juce::DrawableButton::ButtonStyle::ImageOnButtonBackground));
        fButton->setImages(fButtonIcon.get());
        fButton->onClick = [this]() {
          onRemoveButtonClicked(fFile);
        };
        addAndMakeVisible(*fButton);
      }
    }

    void mouseDown(juce::MouseEvent const &e) override {
      fMouseDown = true;
    }

    void mouseUp(juce::MouseEvent const &e) override {
      fMouseDown = false;
    }

    void mouseEnter(juce::MouseEvent const &e) override {
      fMouseOver = true;
    }

    void mouseExit(juce::MouseEvent const &e) override {
      fMouseOver = false;
    }

    void paint(juce::Graphics &g) override {
      {
        g.saveState();
        defer {
          g.restoreState();
        };
        getLookAndFeel().drawConcertinaPanelHeader(g, getBounds(), fMouseOver, fMouseDown, *fParent, *this);
      }
      {
        g.saveState();
        defer {
          g.restoreState();
        };
        g.setColour(juce::Colours::white);
        int const margin = 10;
        g.drawFittedText(fTitle, margin, 0, getWidth() - 2 * margin, getHeight(), juce::Justification::centredLeft, 1);
      }
    }

    void resized() override {
      int const width = getWidth();
      int const height = getHeight();
      int const margin = (height - kButtonSize) / 2;
      if (fButton) {
        fButton->setBounds(width - margin - kButtonSize, margin, kButtonSize, kButtonSize);
      }
    }

  public:
    std::function<void(juce::File)> onRemoveButtonClicked;

  private:
    static int constexpr kButtonSize = 20;

    bool fMouseDown;
    bool fMouseOver;
    juce::ConcertinaPanel *const fParent;
    juce::String fTitle;
    std::unique_ptr<juce::DrawableButton> fButton;
    std::unique_ptr<juce::Drawable> fButtonIcon;
    juce::File fFile;
  };

public:
  BrowserComponent() {
    fConstrainer.reset(new juce::ComponentBoundsConstrainer());
    fConstrainer->setMinimumWidth(100);
    fResizer.reset(new juce::ResizableEdgeComponent(this, fConstrainer.get(), juce::ResizableEdgeComponent::Edge::rightEdge));
    addAndMakeVisible(*fResizer);

    fPanel.reset(new juce::ConcertinaPanel());
    addAndMakeVisible(*fPanel);

    fAddButtonImage.reset(juce::Drawable::createFromImageData(BinaryData::baseline_add_white_18dp_png,
                                                              BinaryData::baseline_add_white_18dp_pngSize)
                              .release());
    fAddButton.reset(new juce::DrawableButton("Add", juce::DrawableButton::ButtonStyle::ImageOnButtonBackground));
    fAddButton->setImages(fAddButtonImage.get());
    addAndMakeVisible(*fAddButton);
    fAddButton->onClick = [this]() {
      browse();
    };

    setSize(kDefaultWidth, 400);
  }

  ~BrowserComponent() override {}

  void resized() override {
    if (!fResizer) {
      return;
    }

    int const width = getWidth();
    int const height = getHeight();
    fResizer->setBounds(width - kResizerWidth, 0, kResizerWidth, height);

    int const margin = 10;
    int const buttonSize = 40;
    fPanel->setBounds(0, 0, width - kResizerWidth, height - margin - buttonSize - margin);

    fAddButton->setBounds(margin, height - margin - buttonSize, buttonSize, buttonSize);

    Component *parent = getParentComponent();
    if (parent) {
      parent->childBoundsChanged(this);
    }
  }

  void addDirectory(juce::File directory) {
#if JUCE_MAC
    // userHomeDirectory = $HOME/Library/Containers/com.github.kbinani.mcview/Data
    File library = File::getSpecialLocation(File::userHomeDirectory).getParentDirectory().getParentDirectory().getParentDirectory();
    File saves = library.getChildFile("Application Support").getChildFile("minecraft").getChildFile("saves");
    bool const fixed = directory.getFullPathName() == saves.getFullPathName();
#else
    bool const fixed = directory.getFullPathName() == DefaultMinecraftSaveDirectory().getFullPathName();
#endif
    Header *header = new Header(fPanel.get(), directory, fixed ? "Default" : directory.getFileName(), !fixed);
    DirectoryBrowserComponent *browser = new DirectoryBrowserComponent(directory);
    browser->onSelect = [this](juce::File f) {
      onSelect(f);
    };
    header->onRemoveButtonClicked = [this](juce::File f) {
      removeDirectory(f);
    };
    fPanel->addPanel(fPanel->getNumPanels(), browser, true);
    fPanel->setCustomPanelHeader(browser, header, true);
    fPanel->setPanelHeaderSize(browser, 32);
    fPanel->expandPanelFully(browser, true);
    fBrowsers.add(browser);
    if (onAdd) {
      onAdd(directory);
    }
    fTimerStarted = juce::Time::getCurrentTime();
    startTimerHz(50);
  }

  void browse() {
    juce::String message = TRANS("Select Minecraft \"saves\" directory");
    int flags = juce::FileBrowserComponent::openMode | juce::FileBrowserComponent::canSelectDirectories;
#if JUCE_MAC
    message += "\n" + TRANS("This dialog opens the default location of the \"saves\" directory. If this directory is appropriate, press the \"Open\" button");
    flags = flags | FileBrowserComponent::showsHiddenFiles;
#endif
    fFileChooser.reset(new juce::FileChooser(message, DefaultMinecraftSaveDirectory()));
    fFileChooser->launchAsync(flags, [this](juce::FileChooser const &chooser) {
      juce::File directory = chooser.getResult();
      if (directory == juce::File()) {
        return;
      }
      for (int i = 0; i < fBrowsers.size(); i++) {
        auto b = fBrowsers[i];
        if (b->fDirectory.getFullPathName() == directory.getFullPathName()) {
          return;
        }
      }
      addDirectory(directory);
    });
  }

  std::function<void(juce::File)> onSelect;
  std::function<void(juce::File)> onAdd;
  std::function<void(juce::File)> onRemove;

  static int constexpr kDefaultWidth = 214;

private:
  void removeDirectory(juce::File dir) {
    for (int i = 1; i < fPanel->getNumPanels(); i++) {
      Component *comp = fPanel->getPanel(i);
      DirectoryBrowserComponent *browser = dynamic_cast<DirectoryBrowserComponent *>(comp);
      if (!browser) {
        continue;
      }
      if (dir.getFullPathName() != browser->fDirectory.getFullPathName()) {
        continue;
      }
      fPanel->removePanel(comp);
      fBrowsers.remove(i);
    }
    if (onRemove) {
      onRemove(dir);
    }
    fTimerStarted = juce::Time::getCurrentTime();
    startTimerHz(50);
  }

  void timerCallback() override {
    fPanel->setBounds(0, 0, fPanel->getWidth() - 1, fPanel->getHeight());
    resized();
    auto const elapsed = juce::Time::getCurrentTime() - fTimerStarted;
    if (elapsed.inMilliseconds() > 150) {
      stopTimer();
    }
  }

private:
  std::unique_ptr<juce::ResizableEdgeComponent> fResizer;
  std::unique_ptr<juce::ComponentBoundsConstrainer> fConstrainer;
  std::unique_ptr<juce::ConcertinaPanel> fPanel;
  std::unique_ptr<juce::DrawableButton> fAddButton;
  std::unique_ptr<juce::Drawable> fAddButtonImage;
  std::unique_ptr<juce::FileChooser> fFileChooser;

  juce::Array<DirectoryBrowserComponent *> fBrowsers;
  juce::Time fTimerStarted;

  static int constexpr kResizerWidth = 8;

  JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(BrowserComponent)
};

} // namespace mcview
