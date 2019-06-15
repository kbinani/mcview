#pragma once

#include "../JuceLibraryCode/JuceHeader.h"
#include "DirectoryBrowser.h"

class Browser : public Component {
public:
    Browser();
    ~Browser();
    
    void resized() override;

    void addDirectory(File directory);

    std::function<void(File)> onSelect;
    
private:
    ScopedPointer<ResizableEdgeComponent> fResizer;

    Array<DirectoryBrowser*> fBrowsers;
    
    static int constexpr kDefaultWidth = 214;
    static int constexpr kResizerWidth = 8;
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(Browser)
};
