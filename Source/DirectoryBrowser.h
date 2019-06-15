#pragma once

#include "../JuceLibraryCode/JuceHeader.h"

class DirectoryBrowser;

class DirectoryBrowserModel : public ListBoxModel {
public:
    DirectoryBrowserModel(DirectoryBrowser *parent, File directory, LookAndFeel const& laf);
    int getNumRows() override;
    void paintListBoxItem (int rowNumber, Graphics &g, int width, int height, bool rowIsSelected) override;

    void applyLookAndFeel(LookAndFeel const& laf);
    
    void listBoxItemDoubleClicked (int row, const MouseEvent &) override;
    
private:
    File fDirectory;
    Array<File> fItems;
    Colour fTextColorOff;
    Colour fTextColorOn;
    Colour fBackgroundColorOff;
    Colour fBackgroundColorOn;
    DirectoryBrowser *const fParent;
};


class DirectoryBrowser : public ListBox, private AsyncUpdater {
public:
    DirectoryBrowser(File directory);
    void lookAndFeelChanged() override;

    std::function<void(File)> onSelect;
    
private:
    void handleAsyncUpdate();
    
private :
    File fDirectory;
    ScopedPointer<DirectoryBrowserModel> fModel;
};
