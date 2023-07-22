#pragma once

class TextInputDialog : public juce::Component {
public:
  TextInputDialog() {
    using namespace juce;
    fInputLabel.reset(new Label());
    fInputLabel->setEditable(true);
    String name
#if JUCE_WINDOWS
        = "Yu Gothic UI";
#else
        = "Hiragino Kaku Gothic Pro";
#endif
    Font font(name, 14, 0);
    fInputLabel->setFont(font);
    fInputLabel->setColour(Label::ColourIds::backgroundColourId, Colours::white);
    fInputLabel->setColour(Label::ColourIds::textColourId, Colours::black);
    fInputLabel->setColour(Label::ColourIds::textWhenEditingColourId, Colours::black);
    fInputLabel->setJustificationType(Justification::topLeft);
    addAndMakeVisible(*fInputLabel);
    fOkButton.reset(new TextButton());
    fOkButton->setButtonText("OK");
    fOkButton->onClick = [this]() {
      close(1);
    };
    addAndMakeVisible(*fOkButton);
    fCancelButton.reset(new TextButton());
    fCancelButton->setButtonText(TRANS("Cancel"));
    fCancelButton->onClick = [this]() {
      close(-1);
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

  static std::pair<int, juce::String> show(juce::Component *target, juce::String title, juce::String init) {
    using namespace juce;
    static std::unique_ptr<TextInputDialog> sComponent(new TextInputDialog());
    sComponent->fInputLabel->setText(init, dontSendNotification);
    juce::DialogWindow::showDialog(title, sComponent.get(), target, target->getLookAndFeel().findColour(TextButton::buttonColourId), true);
    return std::make_pair(sComponent->fResultMenuId, sComponent->fInputLabel->getText());
    // TODO:
    TextInputDialog *dialog = new TextInputDialog();
    DialogWindow::LaunchOptions o;
    o.dialogTitle = title;
    o.content.setOwned(dialog);
    o.componentToCentreAround = target;
    o.dialogBackgroundColour = target->getLookAndFeel().findColour(TextButton::buttonColourId);
    o.escapeKeyTriggersCloseButton = true;
    o.useNativeTitleBar = false;
    o.resizable = false;
    o.useBottomRightCornerResizer = false;

    o.launchAsync();
  }

private:
  void close(int result) {
    fResultMenuId = result;
    juce::Component *pivot = this;
    while (pivot) {
      auto *dlg = dynamic_cast<juce::DialogWindow *>(pivot);
      if (dlg) {
        dlg->closeButtonPressed();
        return;
      }
      pivot = pivot->getParentComponent();
    }
  }

private:
  juce::String fMessage;
  juce::String fResult;
  int fResultMenuId = -1;

  std::unique_ptr<juce::Label> fInputLabel;
  std::unique_ptr<juce::TextButton> fOkButton;
  std::unique_ptr<juce::TextButton> fCancelButton;
};
