#pragma once

namespace mcview {

template <class T>
class TextInputDialog : public juce::Component {
  enum {
    kOk = 1,
    kCancel = -1,
  };

public:
  struct Delegate {
    virtual ~Delegate() = default;
    virtual void textInputDialogDidClickOkButton(juce::String input, T data) = 0;
  };

  TextInputDialog(juce::String init, T data, Delegate *delegate) : fData(data), fDelegate(delegate) {
    using namespace juce;
    fInputLabel.reset(new Label());
    fInputLabel->setEditable(true);
    fInputLabel->setText(init, dontSendNotification);
    String name
#if JUCE_WINDOWS
        = "Yu Gothic UI";
#else
        = "Hiragino Kaku Gothic Pro";
#endif
    juce::Font font(juce::FontOptions(name, 16, 0));
    fInputLabel->setFont(font);
    fInputLabel->setColour(Label::ColourIds::backgroundColourId, Colours::white);
    fInputLabel->setColour(Label::ColourIds::textColourId, Colours::black);
    fInputLabel->setColour(Label::ColourIds::textWhenEditingColourId, Colours::black);
    fInputLabel->setJustificationType(Justification::topLeft);
    addAndMakeVisible(*fInputLabel);
    fOkButton.reset(new TextButton());
    fOkButton->setButtonText("OK");
    fOkButton->onClick = [this]() {
      close(kOk);
    };
    addAndMakeVisible(*fOkButton);
    fCancelButton.reset(new TextButton());
    fCancelButton->setButtonText(TRANS("Cancel"));
    fCancelButton->onClick = [this]() {
      close(kCancel);
    };
    addAndMakeVisible(*fCancelButton);
    setSize(300, 160);
  }

  void resized() override {
    int const pad = 20;
    int const width = getWidth();
    int const height = getHeight();
    int const buttonWidth = 100;
    int const buttonHeight = 40;
    int const inputHeight = height - 3 * pad - buttonHeight;

    if (fInputLabel) {
      fInputLabel->setBounds(pad, pad, width - 2 * pad, inputHeight);
    }
    if (fOkButton) {
      fOkButton->setBounds(width - pad - buttonWidth - pad - buttonWidth, pad + inputHeight + pad, buttonWidth, buttonHeight);
    }
    if (fCancelButton) {
      fCancelButton->setBounds(width - pad - buttonWidth, pad + inputHeight + pad, buttonWidth, buttonHeight);
    }
  }

  template <class D>
  static void showAsync(juce::Component *target, juce::String title, juce::String init, D data, Delegate *delegate) {
    using namespace juce;
    TextInputDialog *dialog = new TextInputDialog(init, data, delegate);
    DialogWindow::LaunchOptions o;
    o.dialogTitle = title;
    o.content.setOwned(dialog);
    o.componentToCentreAround = target;
    o.dialogBackgroundColour = target->getLookAndFeel().findColour(ResizableWindow::ColourIds::backgroundColourId);
    o.escapeKeyTriggersCloseButton = true;
    o.useNativeTitleBar = true;
    o.resizable = false;
    o.useBottomRightCornerResizer = false;
    o.launchAsync();
  }

private:
  void close(int result) {
    if (result == kOk) {
      fDelegate->textInputDialogDidClickOkButton(fInputLabel->getText(), fData);
    }

    juce::Component *pivot = this;
    while (pivot) {
      auto *dlg = dynamic_cast<juce::DialogWindow *>(pivot);
      if (dlg) {
        dlg->closeButtonPressed();
        break;
      }
      pivot = pivot->getParentComponent();
    }
  }

private:
  T fData;
  Delegate *const fDelegate;

  std::unique_ptr<juce::Label> fInputLabel;
  std::unique_ptr<juce::TextButton> fOkButton;
  std::unique_ptr<juce::TextButton> fCancelButton;

  JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(TextInputDialog)
};

} // namespace mcview
