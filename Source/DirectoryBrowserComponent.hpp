#pragma once

namespace mcview {

class DirectoryBrowserComponent : public juce::ListBox, public DirectoryBrowserModel::Delegate {
public:
  DirectoryBrowserComponent(juce::File directory, Edition edition)
      : fDirectory(directory), fEdition(edition) {
    fModel.reset(new DirectoryBrowserModel(this, directory, getLookAndFeel()));
    setModel(fModel.get());
  }

  void lookAndFeelChanged() override {
    fModel->applyLookAndFeel(getLookAndFeel());
  }

  void directoryBrowserModelDidSelectDirectory(juce::File directory) override {
    Directory d;
    d.fDirectory = directory;
    d.fEdition = fEdition;
    onSelect(d);
  }

  std::function<void(Directory)> onSelect;

public:
  juce::File const fDirectory;
  Edition const fEdition;

private:
  std::unique_ptr<DirectoryBrowserModel> fModel;

  JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(DirectoryBrowserComponent)
};

} // namespace mcview
