#pragma once

namespace mcview {

class LeftPanel : public juce::Component, private juce::Timer, public BrowserDelegate {
  enum {
    MenuIdJava = 1,
    MenuIdBedrock = 2,
  };

  enum {
    kPanelHeaderHeight = 32,
  };

public:
  LeftPanel() {
    fConstrainer.reset(new juce::ComponentBoundsConstrainer());
    fConstrainer->setMinimumWidth(kMinimumWidth);
    fResizer.reset(new juce::ResizableEdgeComponent(this, fConstrainer.get(), juce::ResizableEdgeComponent::Edge::rightEdge));
    addAndMakeVisible(*fResizer);

    fPanel.reset(new ConcertinaPanel());
    addAndMakeVisible(*fPanel);

    fCustom.reset(new CustomDirectoryBrowser(this));
    sortPanels();

    fAddButtonImage.reset(juce::Drawable::createFromImageData(BinaryData::baseline_add_white_18dp_png,
                                                              BinaryData::baseline_add_white_18dp_pngSize)
                              .release());
    fAddButton.reset(new ImageButton("Add", juce::DrawableButton::ButtonStyle::ImageOnButtonBackground));
    fAddButton->setImages(fAddButtonImage.get());
    fAddButton->setMouseCursor(juce::MouseCursor::PointingHandCursor);
    addAndMakeVisible(*fAddButton);
    fAddButton->onClick = [this]() {
      onClickAddButton();
    };

    setSize(kDefaultWidth, 400);
  }

  ~LeftPanel() override {}

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

  void addDirectory(Directory directory) {
    bool const java = directory.fDirectory.getFullPathName() == DefaultJavaSaveDirectory().getFullPathName();
    bool const bedrock = directory.fDirectory.getFullPathName() == DefaultBedrockSaveDirectory().getFullPathName();

    if (java) {
      if (fJava) {
        fPanel->removePanel(fJava.get());
      }
      fJava.reset(new GameDirectoryBrowser(directory.fDirectory, directory.fEdition, this));
      sortPanels();
    } else if (bedrock) {
      if (fBedrock) {
        fPanel->removePanel(fBedrock.get());
      }
      fBedrock.reset(new GameDirectoryBrowser(directory.fDirectory, directory.fEdition, this));
      sortPanels();
    } else {
      fCustom->addDirectory(directory);
    }
    updatePanelSizes();
    if (onAdd) {
      onAdd(directory);
    }
    fTimerStarted = juce::Time::getCurrentTime();
    startTimerHz(50);
  }

  void browserDidSelectDirectory(Directory d) override {
    onSelect(d);
  }

  void browserDidClickRemoveDirectory(Directory d) override {
    onRemove(d);
  }

  void browseDefaultJavaSaves() {
    juce::String message = TRANS("Select \"saves\" directory of Minecraft Java Edition");
    int flags = juce::FileBrowserComponent::openMode | juce::FileBrowserComponent::canSelectDirectories;
    message += "\n" + TRANS("This dialog opens the default location of the \"saves\" directory. If this directory is appropriate, press the \"Open\" button");
#if JUCE_MAC
    flags = flags | juce::FileBrowserComponent::showsHiddenFiles;
#endif
    fFileChooser.reset(new juce::FileChooser(message, DefaultJavaSaveDirectory()));
    fFileChooser->launchAsync(flags, [this](juce::FileChooser const &chooser) {
      juce::File directory = chooser.getResult();
      if (directory == juce::File()) {
        return;
      }
      Directory d;
      d.fDirectory = directory;
      d.fEdition = Edition::Java;
      addDirectory(d);
    });
  }

  std::function<void(Directory)> onSelect;
  std::function<void(Directory)> onAdd;
  std::function<void(Directory)> onRemove;

  static int constexpr kDefaultWidth = 214;
  static int constexpr kMinimumWidth = 100;

private:
  void onClickAddButton() {
    juce::PopupMenu menu;
    menu.addItem(MenuIdJava, TRANS("Register Java world directory"));
    menu.addItem(MenuIdBedrock, TRANS("Register Bedrock world directory"));
    menu.showMenuAsync(juce::PopupMenu::Options().withTargetComponent(*fAddButton), [this](int id) {
      switch (id) {
      case MenuIdJava:
        browseJavaWorld();
        break;
      case MenuIdBedrock:
        browseBedrockWorld();
        break;
      }
    });
  }

  void sortPanels() {
    while (fPanel->getNumPanels() > 0) {
      fPanel->removePanel(fPanel->getPanel(0));
    }
    if (fJava) {
      addPanel(fJava.get(), TRANS("Java"));
    }
    if (fBedrock) {
      addPanel(fBedrock.get(), TRANS("Bedrock"));
    }
    if (fCustom) {
      addPanel(fCustom.get(), TRANS("Other"));
    }
  }

  void updatePanelSizes() {
    int numPanels = 0;
    int numItems = 0;
    if (fJava) {
      numPanels++;
      if (fJava->getListBoxModel()->getNumRows() > 0) {
        numItems++;
      }
    }
    if (fBedrock) {
      numPanels++;
      if (fBedrock->getListBoxModel()->getNumRows() > 0) {
        numItems++;
      }
    }
    if (fCustom) {
      numPanels++;
      if (fCustom->getListBoxModel()->getNumRows() > 0) {
        numItems++;
      }
    }
    if (numItems > 0) {
      int availableHeight = fPanel->getHeight();
      if (fJava) {
        if (fJava->getListBoxModel()->getNumRows() > 0) {
          fPanel->setPanelSize(fJava.get(), availableHeight / numItems, false);
        } else {
          fPanel->setPanelSize(fJava.get(), 0, false);
        }
      }
      if (fBedrock) {
        if (fBedrock->getListBoxModel()->getNumRows() > 0) {
          fPanel->setPanelSize(fBedrock.get(), availableHeight / numItems, false);
        } else {
          fPanel->setPanelSize(fBedrock.get(), 0, false);
        }
      }
      if (fCustom) {
        if (fCustom->getListBoxModel()->getNumRows() > 0) {
          fPanel->setPanelSize(fCustom.get(), availableHeight / numItems, false);
        } else {
          fPanel->setPanelSize(fCustom.get(), 0, false);
        }
      }
    }
  }

  void addPanel(juce::Component *panel, juce::String const &headerTitle) {
    LeftPanelHeader *header = new LeftPanelHeader(headerTitle);
    fPanel->addPanel(fPanel->getNumPanels(), panel, false);
    fPanel->setCustomPanelHeader(panel, header, true);
    fPanel->setPanelHeaderSize(panel, kPanelHeaderHeight);
  }

  void browseJavaWorld() {
    juce::String message = TRANS("Select world directory of Minecraft Java Edition");
    int flags = juce::FileBrowserComponent::openMode | juce::FileBrowserComponent::canSelectDirectories;
    fFileChooser.reset(new juce::FileChooser(message));
    fFileChooser->launchAsync(flags, [this](juce::FileChooser const &chooser) {
      juce::File directory = chooser.getResult();
      if (directory == juce::File()) {
        return;
      }
      Directory d;
      d.fDirectory = directory;
      d.fEdition = Edition::Java;
      addDirectory(d);
    });
  }

  void browseBedrockWorld() {
    juce::String message = TRANS("Select world directory of Minecraft Bedrock Edition");
    int flags = juce::FileBrowserComponent::openMode | juce::FileBrowserComponent::canSelectDirectories;
    fFileChooser.reset(new juce::FileChooser(message));
    fFileChooser->launchAsync(flags, [this](juce::FileChooser const &chooser) {
      juce::File directory = chooser.getResult();
      if (directory == juce::File()) {
        return;
      }
      Directory d;
      d.fDirectory = directory;
      d.fEdition = Edition::Bedrock;
      addDirectory(d);
    });
  }

  void removeDirectory(Directory d) {
    if (!fCustom) {
      return;
    }
    fCustom->removeDirectory(d);
    if (onRemove) {
      onRemove(d);
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
  std::unique_ptr<ConcertinaPanel> fPanel;
  std::unique_ptr<ImageButton> fAddButton;
  std::unique_ptr<juce::Drawable> fAddButtonImage;
  std::unique_ptr<juce::FileChooser> fFileChooser;

  std::unique_ptr<GameDirectoryBrowser> fJava;
  std::unique_ptr<GameDirectoryBrowser> fBedrock;
  std::unique_ptr<CustomDirectoryBrowser> fCustom;
  juce::Time fTimerStarted;

  static int constexpr kResizerWidth = 8;

  JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(LeftPanel)
};

} // namespace mcview
