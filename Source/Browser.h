#pragma once

#include "../JuceLibraryCode/JuceHeader.h"

class Browser : public Component {
public:
    Browser();
    
    void resized() override;
    
private:
    ScopedPointer<ResizableEdgeComponent> fResizer;

    static int constexpr kDefaultWidth = 214;
    static int constexpr kResizerWidth = 8;
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(Browser)
};
