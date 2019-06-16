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

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(DirectoryBrowserModel);
};


class DirectoryBrowser : public ListBox {
public:
    DirectoryBrowser(File directory);
    void lookAndFeelChanged() override;

    std::function<void(File)> onSelect;
    
private :
    File fDirectory;
    ScopedPointer<DirectoryBrowserModel> fModel;
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(DirectoryBrowser);
};
