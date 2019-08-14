#pragma once

#include "../JuceLibraryCode/JuceHeader.h"
#include "WorldData.h"

class PinComponent : public Component {
public:
    explicit PinComponent(std::shared_ptr<Pin> const& pin);

    void paint(Graphics &g) override;
    void mouseUp(MouseEvent const&) override;

    void updatePinPosition(Point<float> pos);
    bool isPresenting(std::shared_ptr<Pin> const& p) const
    {
        return p.get() == fPin.get();
    }
    Point<float> getMapCoordinate() const
    {
        return Point<float>(fPin->fX + 0.5f, fPin->fZ - 0.5f);
    }

public:
    std::function<void(std::shared_ptr<Pin>, Point<int> screenPos)> onRightClick;

private:
    std::shared_ptr<Pin> fPin;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(PinComponent)
};
