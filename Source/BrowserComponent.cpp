#include "BrowserComponent.h"
#include "LookAndFeel.h"
#include "defer.h"
#include "MainComponent.h"

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
            fButtonIcon.reset(Drawable::createFromImageData(BinaryData::baseline_remove_white_18dp_png,
                                                            BinaryData::baseline_remove_white_18dp_pngSize).release());
            fButton.reset(new DrawableButton("", DrawableButton::ButtonStyle::ImageOnButtonBackground));
            fButton->setImages(fButtonIcon.get());
            fButton->onClick = [this]() {
                onRemoveButtonClicked(fFile);
            };
            addAndMakeVisible(*fButton);
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
    std::unique_ptr<DrawableButton> fButton;
    std::unique_ptr<Drawable> fButtonIcon;
    File fFile;
};

BrowserComponent::BrowserComponent()
{
    fConstrainer.reset(new ComponentBoundsConstrainer());
    fConstrainer->setMinimumWidth(100);
    fResizer.reset(new ResizableEdgeComponent(this, fConstrainer.get(), ResizableEdgeComponent::Edge::rightEdge));
    addAndMakeVisible(*fResizer);
    
    fPanel.reset(new ConcertinaPanel());
    addAndMakeVisible(*fPanel);
    
    fAddButtonImage.reset(Drawable::createFromImageData(BinaryData::baseline_add_white_18dp_png,
                                                        BinaryData::baseline_add_white_18dp_pngSize).release());
    fAddButton.reset(new DrawableButton("Add", DrawableButton::ButtonStyle::ImageOnButtonBackground));
    fAddButton->setImages(fAddButtonImage.get());
    addAndMakeVisible(*fAddButton);
    fAddButton->onClick = [this]() {
        browse();
    };

    setSize(kDefaultWidth, 400);
}

void BrowserComponent::browse()
{
    String message = TRANS("Select Minecraft \"saves\" directory");
#if JUCE_MAC
    message += String(":\n/Users/yourname/Library/Application Support/minecraft/saves");
#endif
    fFileChooser.reset(new FileChooser(message));
    int const flags = FileBrowserComponent::openMode | FileBrowserComponent::canSelectDirectories | FileBrowserComponent::showsHiddenFiles;
    fFileChooser->launchAsync(flags, [this](FileChooser const& chooser){
        File directory = chooser.getResult();
        if (directory == File()) {
            return;
        }
        for (int i = 0; i < fBrowsers.size(); i++) {
            auto b = fBrowsers[i];
            if (b->fDirectory.getFullPathName() == directory.getFullPathName()) {
                return;
            }
        }
        addDirectory(directory);
    });
}

BrowserComponent::~BrowserComponent()
{
}

void BrowserComponent::addDirectory(File directory)
{
#if JUCE_MAC
    // userHomeDirectory = $HOME/Library/Containers/com.github.kbinani.mcview/Data
    File library = File::getSpecialLocation(File::userHomeDirectory).getParentDirectory().getParentDirectory().getParentDirectory();
    File saves = library.getChildFile("Application Support").getChildFile("minecraft").getChildFile("saves");
    bool const fixed = directory.getFullPathName() == saves.getFullPathName();
#else
    bool const fixed = directory.getFullPathName() == MainComponent::DefaultMinecraftSaveDirectory().getFullPathName();
#endif
    Header *header = new Header(fPanel.get(), directory, fixed ? "Default" : directory.getFileName(), !fixed);
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
