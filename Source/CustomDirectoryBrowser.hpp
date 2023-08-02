#pragma once

namespace mcview {

class CustomDirectoryBrowser : public juce::ListBox, private juce::ListBoxModel {
public:
  explicit CustomDirectoryBrowser(BrowserDelegate *delegate) : fDelegate(delegate) {
    setModel(this);
  }

  int getNumRows() override {
    return (int)fItems.size();
  }

  void paintListBoxItem(int rowNumber, juce::Graphics &g, int width, int height, bool rowIsSelected) override {
    g.saveState();
    defer {
      g.restoreState();
    };

    auto const &laf = getLookAndFeel();
    auto textColorOn = laf.findColour(juce::TextButton::ColourIds::textColourOnId);
    auto textColorOff = laf.findColour(juce::TextButton::ColourIds::textColourOffId);
    auto backgroundColorOn = laf.findColour(juce::TextButton::ColourIds::buttonOnColourId);
    auto backgroundColorOff = laf.findColour(juce::TextButton::ColourIds::buttonColourId);

    int margin = 10;
    g.setColour(rowIsSelected ? backgroundColorOn : backgroundColorOff);
    g.fillRect(0, 0, width, height);

    Directory const &d = fItems[rowNumber];
    juce::String name = d.fDirectory.getFileName();
    switch (d.fEdition) {
    case Edition::Java:
      name += " (Java)";
      g.setColour(juce::Colours::green);
      break;
    case Edition::Bedrock:
      name += " (Bedrock)";
      g.setColour(juce::Colours::lightgrey);
      break;
    }
    int radius = (height - 2 * margin) / 2;
    float cx = margin + radius;
    float cy = height * 0.5f;
    int actualRadius = height * 0.5f / 2;
    g.fillEllipse(cx - actualRadius, cy - actualRadius, actualRadius * 2, actualRadius * 2);
    g.setColour(rowIsSelected ? textColorOn : textColorOff);
    g.drawFittedText(name, margin + radius * 2 + margin, 0, width - 3 * margin - radius * 2, height, juce::Justification::centredLeft, 1);
  }

  void listBoxItemDoubleClicked(int row, juce::MouseEvent const &) override {
    Directory d = fItems[row];
    fDelegate->browserDidSelectDirectory(d);
  }

  void listBoxItemClicked(int row, juce::MouseEvent const &ev) override {
    if (ev.mods.isRightButtonDown()) {
      juce::PopupMenu menu;
      Directory d = fItems[row];
      menu.addItem(TRANS("Unregister"), [this, d]() {
        onRemoveMenuClicked(d);
      });
      juce::PopupMenu::Options opt;
      menu.showMenuAsync(opt);
    }
  }

  void addDirectory(Directory d) {
    for (auto const &i : fItems) {
      if (d == i) {
        return;
      }
    }
    fItems.push_back(d);
    updateContent();
  }

  void removeDirectory(Directory d) {
    for (int i = (int)fItems.size() - 1; i >= 0; i--) {
      if (fItems[i] == d) {
        fItems.erase(fItems.begin() + i);
      }
    }
    updateContent();
  }

private:
  void onRemoveMenuClicked(Directory d) {
    auto options = juce::MessageBoxOptions()
                       .withButton(TRANS("Unregister"))
                       .withButton(TRANS("Cancel"))
                       .withIconType(juce::MessageBoxIconType::QuestionIcon)
                       .withMessage(TRANS("Do you really want to unregister?"))
                       .withTitle(TRANS("Confirm"));
    juce::AlertWindow::showAsync(options, [this, d](int buttonIndex) {
      if (buttonIndex == 1) {
        removeDirectory(d);
        fDelegate->browserDidClickRemoveDirectory(d);
      }
    });
  }

private:
  BrowserDelegate *const fDelegate;
  std::vector<Directory> fItems;
};

} // namespace mcview
