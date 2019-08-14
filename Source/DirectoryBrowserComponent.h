#pragma once

#include "../JuceLibraryCode/JuceHeader.h"

class DirectoryBrowserComponent;

class DirectoryBrowserModel : public ListBoxModel {
public:
    DirectoryBrowserModel(DirectoryBrowserComponent *parent, File directory, LookAndFeel const& laf);
    int getNumRows() override;
    void paintListBoxItem (int rowNumber, Graphics &g, int width, int height, bool rowIsSelected) override;

    void applyLookAndFeel(LookAndFeel const& laf);
    
    void listBoxItemDoubleClicked (int row, const MouseEvent &) override;
    
private:
    File const fDirectory;
    Array<File> fItems;
    Colour fTextColorOff;
    Colour fTextColorOn;
    Colour fBackgroundColorOff;
    Colour fBackgroundColorOn;
    DirectoryBrowserComponent *const fParent;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(DirectoryBrowserModel);
};


class DirectoryBrowserComponent : public ListBox {
public:
    DirectoryBrowserComponent(File directory);
    void lookAndFeelChanged() override;

    std::function<void(File)> onSelect;
    
public:
    File const fDirectory;

private:
    std::unique_ptr<DirectoryBrowserModel> fModel;
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(DirectoryBrowserComponent);
};
