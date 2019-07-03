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
    ScopedPointer<ResizableEdgeComponent> fResizer;
    ScopedPointer<ComponentBoundsConstrainer> fConstrainer;
    ScopedPointer<ConcertinaPanel> fPanel;
    ScopedPointer<DrawableButton> fAddButton;
    ScopedPointer<Drawable> fAddButtonImage;
    
    Array<DirectoryBrowserComponent *> fBrowsers;
    Time fTimerStarted;
    
    static int constexpr kResizerWidth = 8;
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(BrowserComponent)
};
