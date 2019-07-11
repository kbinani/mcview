#pragma once

#include "../JuceLibraryCode/JuceHeader.h"

class AboutComponent : public Component {
public:
    AboutComponent();
    void paint(Graphics &g) override;

private:
    ScopedPointer<Drawable> fLogo;
    Array<ScopedPointer<Component>> fLines;
};
