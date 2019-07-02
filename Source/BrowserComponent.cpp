#include "BrowserComponent.h"
#include "LookAndFeel.h"
#include "defer.h"

class Header : public Component
{
public:
    Header(ConcertinaPanel *parent, File file, String const& title, bool removable)
        : fMouseDown(false)
        , fMouseOver(false)
        , fParent(parent)
        , fTitle(title)
        , fFile(file)
    {
        if (removable) {
            fButtonIcon = Drawable::createFromImageData(BinaryData::baseline_remove_white_18dp_png, BinaryData::baseline_remove_white_18dp_pngSize);
            fButton = new DrawableButton("", DrawableButton::ButtonStyle::ImageOnButtonBackground);
            fButton->setImages(fButtonIcon);
            fButton->onClick = [this]() {
                onRemoveButtonClicked(fFile);
            };
            addAndMakeVisible(fButton);
        }
    }
    
    void mouseDown(MouseEvent const& e) override
    {
        fMouseDown = true;
    }
    
    void mouseUp(MouseEvent const& e) override
    {
        fMouseDown = false;
    }
    
    void mouseEnter(MouseEvent const& e) override
    {
        fMouseOver = true;
    }
    
    void mouseExit(MouseEvent const& e) override
    {
        fMouseOver = false;
    }
    
    void paint(Graphics &g) override
    {
        {
            g.saveState();
            defer {
                g.restoreState();
            };
            getLookAndFeel().drawConcertinaPanelHeader(g, getBounds(), fMouseOver, fMouseDown, *fParent, *this);
        }
        {
            g.saveState();
            defer {
                g.restoreState();
            };
            g.setColour(Colours::white);
            int const margin = 10;
            GraphicsHelper::DrawFittedText(g, fTitle, margin, 0, getWidth() - 2 * margin, getHeight(), Justification::centredLeft, 1);
        }
    }

    void resized() override
    {
        int const width = getWidth();
        int const height = getHeight();
        int const margin = (height - kButtonSize) / 2;
        if (fButton) {
            fButton->setBounds(width - margin - kButtonSize, margin, kButtonSize, kButtonSize);
        }
    }
    
public:
    std::function<void(File)> onRemoveButtonClicked;
    
private:
    static int constexpr kButtonSize = 20;
    
    bool fMouseDown;
    bool fMouseOver;
    ConcertinaPanel *const fParent;
    String fTitle;
    ScopedPointer<DrawableButton> fButton;
    ScopedPointer<Drawable> fButtonIcon;
    File fFile;
};

BrowserComponent::BrowserComponent()
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

BrowserComponent::~BrowserComponent()
{
}

void BrowserComponent::addDirectory(File directory, bool fixed)
{
    Header *header = new Header(fPanel, directory, fixed ? "Default" : directory.getFileName(), !fixed);
    DirectoryBrowserComponent* browser = new DirectoryBrowserComponent(directory);
    browser->onSelect = [this](File f) {
        onSelect(f);
    };
    header->onRemoveButtonClicked = [this](File f) {
        removeDirectory(f);
    };
    fPanel->addPanel(fPanel->getNumPanels(), browser, true);
    fPanel->setCustomPanelHeader(browser, header, true);
    fPanel->setPanelHeaderSize(browser, 32);
    fPanel->expandPanelFully(browser, true);
    fBrowsers.add(browser);
    if (onAdd) {
        onAdd(directory);
    }
    fTimerStarted = Time::getCurrentTime();
    startTimerHz(50);
}

void BrowserComponent::removeDirectory(File dir)
{
    for (int i = 1; i < fPanel->getNumPanels(); i++) {
        Component *comp = fPanel->getPanel(i);
        DirectoryBrowserComponent *browser = dynamic_cast<DirectoryBrowserComponent *>(comp);
        if (!browser) {
            continue;
        }
        if (dir.getFullPathName() != browser->fDirectory.getFullPathName()) {
            continue;
        }
        fPanel->removePanel(comp);
        fBrowsers.remove(i);
    }
    if (onRemove) {
        onRemove(dir);
    }
    fTimerStarted = Time::getCurrentTime();
    startTimerHz(50);
}

void BrowserComponent::resized()
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

void BrowserComponent::timerCallback()
{
    fPanel->setBounds(0, 0, fPanel->getWidth() - 1, fPanel->getHeight());
    resized();
    auto const elapsed = Time::getCurrentTime() - fTimerStarted;
    if (elapsed.inMilliseconds() > 150) {
        stopTimer();
    }
}
