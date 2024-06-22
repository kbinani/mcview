#pragma once

namespace mcview {

class CustomDirectoryBrowser : public juce::ListBox, private juce::ListBoxModel {
public:
  explicit CustomDirectoryBrowser(BrowserDelegate *delegate) : fDelegate(delegate) {
    setRowHeight(60);
    setModel(this);
  }

  int getNumRows() override {
    return (int)fItems.size();
  }

  void paintListBoxItem(int rowNumber, juce::Graphics &g, int width, int height, bool rowIsSelected) override {
    auto const &laf = getLookAndFeel();
    auto textColorOn = laf.findColour(juce::TextButton::ColourIds::textColourOnId);
    auto textColorOff = laf.findColour(juce::TextButton::ColourIds::textColourOffId);
    auto backgroundColorOn = laf.findColour(juce::TextButton::ColourIds::buttonOnColourId);
    auto backgroundColorOff = laf.findColour(juce::TextButton::ColourIds::buttonColourId);
    fItems[rowNumber].draw(g, juce::Rectangle<int>(0, 0, width, height), rowIsSelected, false, textColorOn, textColorOff, backgroundColorOn, backgroundColorOff);
  }

  void listBoxItemDoubleClicked(int row, juce::MouseEvent const &) override {
    GameDirectory const &d = fItems[row];
    fDelegate->browserDidSelectDirectory(d);
  }

  void listBoxItemClicked(int row, juce::MouseEvent const &ev) override {
    if (ev.mods.isRightButtonDown()) {
      juce::PopupMenu menu;
      GameDirectory d = fItems[row];
      menu.addItem(TRANS("Unregister"), [this, d]() {
        onRemoveMenuClicked(d);
      });
      juce::PopupMenu::Options opt;
      menu.showMenuAsync(opt);
    }
  }

  std::optional<GameDirectory> addDirectory(juce::File d) {
    for (auto const &i : fItems) {
      if (d == i.fDirectory) {
        return std::nullopt;
      }
    }
    if (auto gd = GameDirectory::Open(d); gd) {
      fItems.push_back(*gd);
      updateContent();
      return gd;
    } else {
      return std::nullopt;
    }
  }

  void removeDirectory(GameDirectory d) {
    for (int i = (int)fItems.size() - 1; i >= 0; i--) {
      if (fItems[i].fDirectory == d.fDirectory) {
        fItems.erase(fItems.begin() + i);
      }
    }
    updateContent();
  }

private:
  void onRemoveMenuClicked(GameDirectory d) {
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
  std::vector<GameDirectory> fItems;
};

} // namespace mcview
