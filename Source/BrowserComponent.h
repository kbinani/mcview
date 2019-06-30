#pragma once

#include "../JuceLibraryCode/JuceHeader.h"
#include "DirectoryBrowserComponent.h"

class BrowserComponent : public Component {
public:
    BrowserComponent();
    ~BrowserComponent();
    
    void resized() override;

    void addDirectory(File directory, String title = String());

    std::function<void(File)> onSelect;
    std::function<void(File)> onAdd;

    static int constexpr kDefaultWidth = 214;

private:
    ScopedPointer<ResizableEdgeComponent> fResizer;
    ScopedPointer<ComponentBoundsConstrainer> fConstrainer;
    ScopedPointer<ConcertinaPanel> fPanel;
    ScopedPointer<DrawableButton> fAddButton;
    ScopedPointer<Drawable> fAddButtonImage;
    
    Array<DirectoryBrowserComponent *> fBrowsers;
    
    static int constexpr kResizerWidth = 8;
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(BrowserComponent)
};
