#include "Browser.h"

Browser::Browser()
{
    fConstrainer = new ComponentBoundsConstrainer();
    fConstrainer->setMinimumWidth(100);
    fResizer = new ResizableEdgeComponent(this, fConstrainer, ResizableEdgeComponent::Edge::rightEdge);
    addAndMakeVisible(fResizer);
    
    fPanel = new ConcertinaPanel();
    addAndMakeVisible(fPanel);
    
    setSize(kDefaultWidth, 400);
}

Browser::~Browser()
{
}

void Browser::addDirectory(File directory, String title)
{
    DirectoryBrowser* browser = new DirectoryBrowser(directory);
    browser->onSelect = [this](File f) {
        onSelect(f);
    };
    browser->setName(title.length() == 0 ? directory.getFileName() : title);
    fPanel->addPanel(fPanel->getNumPanels(), browser, true);
    fPanel->setPanelHeaderSize(browser, 32);
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
    
    fPanel->setBounds(0, 0, width - kResizerWidth, height);
    
    Component *parent = getParentComponent();
    if (parent) {
        parent->childBoundsChanged(this);
    }
}
