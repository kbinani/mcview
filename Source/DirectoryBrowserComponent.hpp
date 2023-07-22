#pragma once

namespace mcview {

class DirectoryBrowserModel : public juce::ListBoxModel {
public:
  class Delegate {
  public:
    virtual ~Delegate() = default;
    virtual void directoryBrowserModelDidSelectDirectory(juce::File directory) = 0;
  };

  DirectoryBrowserModel(Delegate *delegate, juce::File directory, juce::LookAndFeel const &laf)
      : fDirectory(directory), fDelegate(delegate) {
    juce::DirectoryIterator it(fDirectory, false, "*", juce::File::findDirectories);
    while (it.next()) {
      juce::File file = it.getFile();
      if (!file.isDirectory()) {
        continue;
      }
      fItems.add(file);
    }
    fItems.sort();

    applyLookAndFeel(laf);
  }

  int getNumRows() override {
    return fItems.size();
  }

  void paintListBoxItem(int rowNumber, juce::Graphics &g, int width, int height, bool rowIsSelected) override {
    g.saveState();
    defer {
      g.restoreState();
    };

    int margin = 10;
    g.setColour(rowIsSelected ? fBackgroundColorOn : fBackgroundColorOff);
    g.fillRect(0, 0, width, height);
    g.setColour(rowIsSelected ? fTextColorOn : fTextColorOff);

    juce::String const name = fItems[rowNumber].getFileName();
    GraphicsHelper::DrawFittedText(g, name, margin, 0, width - 2 * margin, height, juce::Justification::centredLeft, 1);
  }

  void applyLookAndFeel(juce::LookAndFeel const &laf) {
    fTextColorOn = laf.findColour(juce::TextButton::ColourIds::textColourOnId);
    fTextColorOff = laf.findColour(juce::TextButton::ColourIds::textColourOffId);
    fBackgroundColorOn = laf.findColour(juce::TextButton::ColourIds::buttonOnColourId);
    fBackgroundColorOff = laf.findColour(juce::TextButton::ColourIds::buttonColourId);
  }

  void listBoxItemDoubleClicked(int row, juce::MouseEvent const &) override {
    juce::File dir = fItems[row];
    fDelegate->directoryBrowserModelDidSelectDirectory(dir);
  }

private:
  juce::File const fDirectory;
  juce::Array<juce::File> fItems;
  juce::Colour fTextColorOff;
  juce::Colour fTextColorOn;
  juce::Colour fBackgroundColorOff;
  juce::Colour fBackgroundColorOn;
  Delegate *const fDelegate;

  JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(DirectoryBrowserModel);
};

class DirectoryBrowserComponent : public juce::ListBox, public DirectoryBrowserModel::Delegate {
public:
  DirectoryBrowserComponent(juce::File directory)
      : fDirectory(directory) {
    fModel.reset(new DirectoryBrowserModel(this, directory, getLookAndFeel()));
    setModel(fModel.get());
  }

  void lookAndFeelChanged() override {
    fModel->applyLookAndFeel(getLookAndFeel());
  }

  void directoryBrowserModelDidSelectDirectory(juce::File directory) override {
    onSelect(directory);
  }

  std::function<void(juce::File)> onSelect;

public:
  juce::File const fDirectory;

private:
  std::unique_ptr<DirectoryBrowserModel> fModel;

  JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(DirectoryBrowserComponent);
};

} // namespace mcview
