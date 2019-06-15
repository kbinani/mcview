#include "Browser.h"

Browser::Browser()
{
    fResizer = new ResizableEdgeComponent(this, nullptr, ResizableEdgeComponent::Edge::rightEdge);
    addAndMakeVisible(fResizer);

    setSize(kDefaultWidth, 400);
}

void Browser::resized()
{
    if (!fResizer) {
        return;
    }

    int const width = getWidth();
    int const height = getHeight();
    fResizer->setBounds(width - kResizerWidth, 0, kResizerWidth, height);
    
    Component *parent = getParentComponent();
    if (parent) {
        parent->childBoundsChanged(this);
    }
}
