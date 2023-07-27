#pragma once

namespace mcview {

class MainWindow : public juce::DocumentWindow, public MainComponent::Delegate {
public:
  MainWindow(juce::String name) : juce::DocumentWindow(name,
                                                       juce::Desktop::getInstance().getDefaultLookAndFeel().findColour(juce::ResizableWindow::backgroundColourId),
                                                       juce::DocumentWindow::allButtons) {
    setUsingNativeTitleBar(true);
    fContent.reset(new MainComponent(this));
    setContentNonOwned(fContent.get(), true);

#if JUCE_IOS || JUCE_ANDROID
    setFullScreen(true);
#else
    setResizable(true, true);
    centreWithSize(getWidth(), getHeight());
#endif

    setVisible(true);
  }

  void closeButtonPressed() override {
    if (!isPresentingAboutDialog() && !fContent->isClosing()) {
      fContent->startClosing();
    }
  }

  void mainComponentDidClose() override {
    juce::JUCEApplication::getInstance()->systemRequestedQuit();
  }

private:
  bool isPresentingAboutDialog() const {
    TopLevelWindow *top = TopLevelWindow::getActiveTopLevelWindow();
    if (!top) {
      return false;
    }
    DocumentWindow *doc = dynamic_cast<DocumentWindow *>(top);
    if (!doc) {
      return false;
    }
    Component *content = doc->getContentComponent();
    if (!content) {
      return false;
    }
    AboutComponent *about = dynamic_cast<AboutComponent *>(content);
    if (!about) {
      return false;
    }
    return true;
  }

private:
  std::unique_ptr<MainComponent> fContent;

  JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(MainWindow)
};

} // namespace mcview
