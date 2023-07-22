#pragma once

#include <juce_gui_extra/juce_gui_extra.h>

class DirectoryBrowserComponent;

class DirectoryBrowserModel : public juce::ListBoxModel {
public:
  DirectoryBrowserModel(DirectoryBrowserComponent *parent, juce::File directory, juce::LookAndFeel const &laf);
  int getNumRows() override;
  void paintListBoxItem(int rowNumber, juce::Graphics &g, int width, int height, bool rowIsSelected) override;

  void applyLookAndFeel(juce::LookAndFeel const &laf);

  void listBoxItemDoubleClicked(int row, const juce::MouseEvent &) override;

private:
  juce::File const fDirectory;
  juce::Array<juce::File> fItems;
  juce::Colour fTextColorOff;
  juce::Colour fTextColorOn;
  juce::Colour fBackgroundColorOff;
  juce::Colour fBackgroundColorOn;
  DirectoryBrowserComponent *const fParent;

  JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(DirectoryBrowserModel);
};

class DirectoryBrowserComponent : public juce::ListBox {
public:
  DirectoryBrowserComponent(juce::File directory);
  void lookAndFeelChanged() override;

  std::function<void(juce::File)> onSelect;

public:
  juce::File const fDirectory;

private:
  std::unique_ptr<DirectoryBrowserModel> fModel;

  JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(DirectoryBrowserComponent);
};
