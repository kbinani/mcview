#pragma once

#include <juce_gui_extra/juce_gui_extra.h>

class Settings {
public:
    Settings();
    
    juce::Array<juce::File> directories() const { return fDirectories; }
    void addDirectory(juce::File f);
    void removeDirectory(juce::File f);

    void load();
    void save();

    static juce::File ConfigDirectory();

public:
    float fWaterOpticalDensity;
    bool fWaterTranslucent = true;
    bool fBiomeEnabled = true;
    int fBiomeBlend;
    bool fShowPin = true;

private:
    static juce::File ConfigFile();
    
    juce::Array<juce::File> fDirectories;

#if JUCE_MAC
    static juce::String Bookmark(juce::File f);
    static juce::File FromBookmark(juce::String s);
#endif
};
