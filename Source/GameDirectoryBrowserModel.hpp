#pragma once

namespace mcview {

class GameDirectoryBrowserModel : public juce::ListBoxModel {
public:
  class Delegate {
  public:
    virtual ~Delegate() = default;
    virtual void directoryBrowserModelDidSelectDirectory(GameDirectory const &item) = 0;
  };

  GameDirectoryBrowserModel(Delegate *delegate, juce::File directory, juce::LookAndFeel const &laf)
      : fDirectory(directory), fDelegate(delegate) {

    for (auto const &it : juce::RangedDirectoryIterator(fDirectory, false, "*", juce::File::findDirectories)) {
      juce::File file = it.getFile();
      if (!file.isDirectory()) {
        continue;
      }
      if (auto item = GameDirectory::Open(file); item) {
        fItems.push_back(*item);
      }
    }
    std::sort(fItems.begin(), fItems.end(), GameDirectory::Compare);

    applyLookAndFeel(laf);
  }

  int getNumRows() override {
    return fItems.size();
  }

  void paintListBoxItem(int rowNumber, juce::Graphics &g, int width, int height, bool rowIsSelected) override {
    fItems[rowNumber].draw(g, juce::Rectangle<int>(0, 0, width, height), rowIsSelected, true, fTextColorOn, fTextColorOff, fBackgroundColorOn, fBackgroundColorOff);
  }

  void applyLookAndFeel(juce::LookAndFeel const &laf) {
    fTextColorOn = laf.findColour(juce::TextButton::ColourIds::textColourOnId);
    fTextColorOff = laf.findColour(juce::TextButton::ColourIds::textColourOffId);
    fBackgroundColorOn = laf.findColour(juce::TextButton::ColourIds::buttonOnColourId);
    fBackgroundColorOff = laf.findColour(juce::TextButton::ColourIds::buttonColourId);
  }

  void listBoxItemDoubleClicked(int row, juce::MouseEvent const &) override {
    auto item = fItems[row];
    fDelegate->directoryBrowserModelDidSelectDirectory(item);
  }

  juce::String getTooltipForRow(int row) override {
    return fItems[row].fDirectory.getFullPathName();
  }

  void listBoxItemClicked(int row, juce::MouseEvent const &e) override {
    if (e.mods.isRightButtonDown()) {
      juce::PopupMenu menu;
      juce::String title;
#if JUCE_WINDOWS
      title = TRANS("Open in Explorer");
#else
      title = TRANS("Open in Finder");
#endif
      int const id = 1;
      menu.addItem(id, title, true, false);
      juce::PopupMenu::Options o;
      menu.showMenuAsync(o, [this, id, row](int ret) {
        if (ret != id) {
          return;
        }
        fItems[row].fDirectory.revealToUser();
      });
    }
  }

private:
  juce::File const fDirectory;
  std::deque<GameDirectory> fItems;
  juce::Colour fTextColorOff;
  juce::Colour fTextColorOn;
  juce::Colour fBackgroundColorOff;
  juce::Colour fBackgroundColorOn;
  Delegate *const fDelegate;

  JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(GameDirectoryBrowserModel)
};

} // namespace mcview
