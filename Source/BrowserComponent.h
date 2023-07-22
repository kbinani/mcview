#pragma once

#include "DirectoryBrowserComponent.h"
#include <juce_gui_extra/juce_gui_extra.h>

class BrowserComponent : public juce::Component, private juce::Timer {
public:
  BrowserComponent();
  ~BrowserComponent();

  void resized() override;

  void addDirectory(juce::File directory);
  void browse();

  std::function<void(juce::File)> onSelect;
  std::function<void(juce::File)> onAdd;
  std::function<void(juce::File)> onRemove;

  static int constexpr kDefaultWidth = 214;

private:
  void removeDirectory(juce::File dir);

  void timerCallback() override;

private:
  std::unique_ptr<juce::ResizableEdgeComponent> fResizer;
  std::unique_ptr<juce::ComponentBoundsConstrainer> fConstrainer;
  std::unique_ptr<juce::ConcertinaPanel> fPanel;
  std::unique_ptr<juce::DrawableButton> fAddButton;
  std::unique_ptr<juce::Drawable> fAddButtonImage;
  std::unique_ptr<juce::FileChooser> fFileChooser;

  juce::Array<DirectoryBrowserComponent *> fBrowsers;
  juce::Time fTimerStarted;

  static int constexpr kResizerWidth = 8;

  JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(BrowserComponent)
};
