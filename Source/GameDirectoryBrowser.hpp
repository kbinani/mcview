#pragma once

namespace mcview {

struct BrowserDelegate {
  virtual ~BrowserDelegate() = default;
  virtual void browserDidSelectDirectory(GameDirectory directory) = 0;
  virtual void browserDidClickRemoveDirectory(GameDirectory directory) = 0;
};

class GameDirectoryBrowser : public juce::ListBox, public GameDirectoryBrowserModel::Delegate {
public:
  GameDirectoryBrowser(juce::File directory, Edition edition, BrowserDelegate *delegate)
      : fDirectory(directory), fEdition(edition), fDelegate(delegate) {
    setRowHeight(60);
    fModel.reset(new GameDirectoryBrowserModel(this, directory, getLookAndFeel()));
    setModel(fModel.get());
  }

  void lookAndFeelChanged() override {
    fModel->applyLookAndFeel(getLookAndFeel());
  }

  void directoryBrowserModelDidSelectDirectory(GameDirectory const &d) override {
    fDelegate->browserDidSelectDirectory(d);
  }

public:
  juce::File const fDirectory;
  Edition const fEdition;

private:
  std::unique_ptr<GameDirectoryBrowserModel> fModel;
  BrowserDelegate *const fDelegate;

  JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(GameDirectoryBrowser)
};

} // namespace mcview
