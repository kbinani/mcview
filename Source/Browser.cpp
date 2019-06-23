#include "Browser.h"

Browser::Browser()
{
    fConstrainer = new ComponentBoundsConstrainer();
    fConstrainer->setMinimumWidth(100);
    fResizer = new ResizableEdgeComponent(this, fConstrainer, ResizableEdgeComponent::Edge::rightEdge);
    addAndMakeVisible(fResizer);
    
    fPanel = new ConcertinaPanel();
    addAndMakeVisible(fPanel);
    
    fAddButtonImage = Drawable::createFromImageData(BinaryData::baseline_add_white_18dp_png,
                                                                BinaryData::baseline_add_white_18dp_pngSize);
    fAddButton = new DrawableButton("Add", DrawableButton::ButtonStyle::ImageOnButtonBackground);
    fAddButton->setImages(fAddButtonImage);
    addAndMakeVisible(fAddButton);
    fAddButton->onClick = [this]() {
        FileChooser dialog("Choose directory");
        if (!dialog.browseForDirectory()) {
            return;
        }
        File directory = dialog.getResult();
        if (directory.getChildFile("region").exists() && directory.getChildFile("level.dat").exists()) {
            directory = directory.getParentDirectory();
        }
        for (int i = 0; i < fBrowsers.size(); i++) {
            auto b = fBrowsers[i];
            if (b->fDirectory.getFullPathName() == directory.getFullPathName()) {
                return;
            }
        }
        addDirectory(directory);
    };

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
    fPanel->expandPanelFully(browser, true);
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
    
    int const margin = 10;
    int const buttonSize = 40;
    fPanel->setBounds(0, 0, width - kResizerWidth, height - margin - buttonSize - margin);
    
    fAddButton->setBounds(margin, height - margin - buttonSize, buttonSize, buttonSize);
    
    Component *parent = getParentComponent();
    if (parent) {
        parent->childBoundsChanged(this);
    }
}
