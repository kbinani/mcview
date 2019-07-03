#pragma once

#include "../JuceLibraryCode/JuceHeader.h"

class Settings {
public:
    Settings();
    
    Array<File> directories() const { return fDirectories; }
    void addDirectory(File f);
    void removeDirectory(File f);

    void load();
    void save();

public:
    float fWaterOpticalDensity;
    bool fWaterTranslucent = true;
    bool fBiomeEnabled = true;
    int fBiomeBlend;

private:
    static File ConfigDirectory();
    static File ConfigFile();
    
    Array<File> fDirectories;

#if JUCE_MAC
    static String Bookmark(File f);
    static File FromBookmark(String s);
#endif
};
