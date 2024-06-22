#pragma once

namespace mcview {

struct BrowserDelegate {
  virtual ~BrowserDelegate() = default;
  virtual void browserDidSelectDirectory(Directory directory) = 0;
  virtual void browserDidClickRemoveDirectory(Directory directory) = 0;
};

class GameDirectoryBrowser : public juce::ListBox, public DirectoryBrowserModel::Delegate {
public:
  GameDirectoryBrowser(juce::File directory, Edition edition, BrowserDelegate *delegate)
      : fDirectory(directory), fEdition(edition), fDelegate(delegate) {
    setRowHeight(60);
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
    fDelegate->browserDidSelectDirectory(d);
  }

public:
  juce::File const fDirectory;
  Edition const fEdition;

private:
  std::unique_ptr<DirectoryBrowserModel> fModel;
  BrowserDelegate *const fDelegate;

  JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(GameDirectoryBrowser)
};

} // namespace mcview
