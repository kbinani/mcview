#pragma once

namespace mcview {

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

  JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(DirectoryBrowserComponent)
};

} // namespace mcview
