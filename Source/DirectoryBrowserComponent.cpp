#include "DirectoryBrowserComponent.h"
#include "defer.h"
#include "GraphicsHelper.h"

DirectoryBrowserModel::DirectoryBrowserModel(DirectoryBrowserComponent *parent, File directory, LookAndFeel const& laf)
    : fDirectory(directory)
    , fParent(parent)
{
    DirectoryIterator it(fDirectory, false, "*", File::findDirectories);
    while (it.next()) {
        File file = it.getFile();
        if (!file.isDirectory()) {
            continue;
        }
        fItems.add(file);
    }
    fItems.sort();
    
    applyLookAndFeel(laf);
}

int DirectoryBrowserModel::getNumRows()
{
    return fItems.size();
}

class Dummy : public TextEditor
{
public:
    void paint(Graphics &g) override
    {
        TextEditor::paint(g);
        TextEditor::paintOverChildren(g);
    }
};

void DirectoryBrowserModel::paintListBoxItem (int rowNumber, Graphics &g, int width, int height, bool rowIsSelected)
{
    g.saveState();
    defer {
        g.restoreState();
    };
    
    
    int margin = 10;
    g.setColour(rowIsSelected ? fBackgroundColorOn : fBackgroundColorOff);
    g.fillRect(0, 0, width, height);
    g.setColour(rowIsSelected ? fTextColorOn : fTextColorOff);

    String const name = fItems[rowNumber].getFileName();
    GraphicsHelper::DrawFittedText(g, name, margin, 0, width - 2 * margin, height, Justification::centredLeft, 1);
}

void DirectoryBrowserModel::applyLookAndFeel(LookAndFeel const& laf)
{
    fTextColorOn = laf.findColour(TextButton::ColourIds::textColourOnId);
    fTextColorOff = laf.findColour(TextButton::ColourIds::textColourOffId);
    fBackgroundColorOn = laf.findColour(TextButton::ColourIds::buttonOnColourId);
    fBackgroundColorOff = laf.findColour(TextButton::ColourIds::buttonColourId);
}

void DirectoryBrowserModel::listBoxItemDoubleClicked (int row, const MouseEvent &)
{
    File dir = fItems[row];
    fParent->onSelect(dir);
}

DirectoryBrowserComponent::DirectoryBrowserComponent(File directory)
    : fDirectory(directory)
{
    fModel.reset(new DirectoryBrowserModel(this, directory, getLookAndFeel()));
    setModel(fModel.get());
}

void DirectoryBrowserComponent::lookAndFeelChanged()
{
    fModel->applyLookAndFeel(getLookAndFeel());
}
