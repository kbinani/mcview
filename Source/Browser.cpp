#include "Browser.h"

Browser::Browser()
{
    fConstrainer = new ComponentBoundsConstrainer();
    fConstrainer->setMinimumWidth(100);
    fResizer = new ResizableEdgeComponent(this, fConstrainer, ResizableEdgeComponent::Edge::rightEdge);
    addAndMakeVisible(fResizer);
    
    setSize(kDefaultWidth, 400);
}

Browser::~Browser()
{
    for (DirectoryBrowser* component : fBrowsers) {
        removeChildComponent(component);
        delete component;
    }
    fBrowsers.clear();
}

void Browser::addDirectory(File directory)
{
    DirectoryBrowser* browser = new DirectoryBrowser(directory);
    browser->onSelect = [this](File f) {
        onSelect(f);
    };
    addAndMakeVisible(browser);
    fBrowsers.add(browser);
    resized();
}

void Browser::resized()
{
    if (!fResizer) {
        return;
    }

    int const width = getWidth();
    int const height = getHeight();
    fResizer->setBounds(width - kResizerWidth, 0, kResizerWidth, height);
    
    int y = 0;
    for (DirectoryBrowser* component : fBrowsers) {
        component->setBounds(0, y, width - kResizerWidth, height);
        y += component->getHeight();
        break; //TODO: multiple directory support
    }
    
    Component *parent = getParentComponent();
    if (parent) {
        parent->childBoundsChanged(this);
    }
}
