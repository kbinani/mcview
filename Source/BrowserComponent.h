#pragma once

#include "../JuceLibraryCode/JuceHeader.h"
#include "DirectoryBrowserComponent.h"

class BrowserComponent : public Component, private Timer {
public:
    BrowserComponent();
    ~BrowserComponent();
    
    void resized() override;

    void addDirectory(File directory);
    void browse();

    std::function<void(File)> onSelect;
    std::function<void(File)> onAdd;
    std::function<void(File)> onRemove;

    static int constexpr kDefaultWidth = 214;

private:
    void removeDirectory(File dir);
    
    void timerCallback() override;
    
private:
    std::unique_ptr<ResizableEdgeComponent> fResizer;
    std::unique_ptr<ComponentBoundsConstrainer> fConstrainer;
    std::unique_ptr<ConcertinaPanel> fPanel;
    std::unique_ptr<DrawableButton> fAddButton;
    std::unique_ptr<Drawable> fAddButtonImage;
    std::unique_ptr<FileChooser> fFileChooser;

    Array<DirectoryBrowserComponent *> fBrowsers;
    Time fTimerStarted;
    
    static int constexpr kResizerWidth = 8;
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(BrowserComponent)
};
