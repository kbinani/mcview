#pragma once

#include "../JuceLibraryCode/JuceHeader.h"
#include "DirectoryBrowser.h"

class Browser : public Component {
public:
    Browser();
    ~Browser();
    
    void resized() override;

    void addDirectory(File directory, String title = String());

    std::function<void(File)> onSelect;

    static int constexpr kDefaultWidth = 214;

private:
    ScopedPointer<ResizableEdgeComponent> fResizer;
    ScopedPointer<ComponentBoundsConstrainer> fConstrainer;
    ScopedPointer<ConcertinaPanel> fPanel;
        
    static int constexpr kResizerWidth = 8;
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(Browser)
};
