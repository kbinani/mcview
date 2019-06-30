#include "Settings.h"
#include "SettingsComponent.h"

static const Identifier kDirectories("directories");
static const Identifier kWaterTranslucent("water_translucent");
static const Identifier kWaterOpticalDensity("water_optical_density");
static const Identifier kBiomeEnabled("biome_enabled");
static const Identifier kBiomeBlend("biome_blend");

File Settings::ConfigDirectory()
{
#if JUCE_WINDOWS
    return File::getSpecialLocation(File::userApplicationDataDirectory).getChildFile(".mcview");
#else
    return File::getSpecialLocation(File::userApplicationDataDirectory).getChildFile("Application Support").getChildFile(".mcview");
#endif
}

File Settings::ConfigFile()
{
    return ConfigDirectory().getChildFile("config.json");
}

Settings::Settings()
    : fWaterOpticalDensity(SettingsComponent::kDefaultWaterAbsorptionCoefficient)
    , fWaterTranslucent(true)
    , fBiomeEnabled(true)
    , fBiomeBlend(SettingsComponent::kDefaultBiomeBlend)
{
}

void Settings::load()
{
    File configFile = ConfigFile();
    FileInputStream stream(configFile);
    if (!stream.openedOk()) {
        return;
    }
    var v = JSON::parse(stream);

    // directories
    if (v.hasProperty(kDirectories)) {
        var dirs = v.getProperty(kDirectories, var());
        if (dirs.isArray()) {
            Array<var> *d = dirs.getArray();
            if (d) {
                for (int i = 0; i < d->size(); i++) {
                    var f = (*d)[i];
                    if (f.isString()) {
                        File ff(f.toString());
                        if (ff.exists() && ff.isDirectory()) {
                            fDirectories.add(ff);
                        }
                    }
                }
            }
        }
    }
    
    // water_optical_density
    if (v.hasProperty(kWaterOpticalDensity)) {
        var vv = v.getProperty(kWaterOpticalDensity, var());
        if (vv.isDouble()) {
            double value = (double)vv;
            fWaterOpticalDensity = std::min(std::max((float)value, SettingsComponent::kMinWaterAbsorptionCoefficient), SettingsComponent::kMaxWaterAbsorptionCoefficient);
        }
    }
    
    // water_translucent
    if (v.hasProperty(kWaterTranslucent)) {
        var vv = v.getProperty(kWaterTranslucent, var());
        if (vv.isBool()) {
            fWaterTranslucent = (bool)vv;
        }
    }
    
    // biome_enabled
    if (v.hasProperty(kBiomeEnabled)) {
        var vv = v.getProperty(kBiomeEnabled, var());
        if (vv.isBool()) {
            fBiomeEnabled = (bool)vv;
        }
    }
    
    // biome_blend
    if (v.hasProperty(kBiomeBlend)) {
        var vv = v.getProperty(kBiomeBlend, var());
        if (vv.isInt()) {
            fBiomeBlend = std::min(std::max((int)vv, SettingsComponent::kMinBiomeBlend), SettingsComponent::kMaxBiomeBlend);
        }
    }
}

void Settings::save()
{
    File dir = ConfigDirectory();
    if (!dir.exists()) {
        dir.createDirectory();
    }
    File configFile = ConfigFile();
    configFile.deleteFile();
    FileOutputStream stream(configFile);
    stream.truncate();
    stream.setPosition(0);

    ScopedPointer<DynamicObject> obj = new DynamicObject();
    
    // directories
    Array<var> dirs;
    for (int i = 0; i < fDirectories.size(); i++) {
        File f = fDirectories[i];
        dirs.add(var(f.getFullPathName()));
    }
    obj->setProperty(kDirectories, var(dirs));

    // water_optical_density
    obj->setProperty(kWaterOpticalDensity, var(fWaterOpticalDensity));
    
    // water_translucent
    obj->setProperty(kWaterTranslucent, var(fWaterTranslucent));
    
    // biome_enabled
    obj->setProperty(kBiomeEnabled, var(fBiomeEnabled));
    
    // biome_blend
    obj->setProperty(kBiomeBlend, var(fBiomeBlend));
    
    obj->writeAsJSON(stream, 4, false, 16);
}

void Settings::addDirectory(File f)
{
    if (fDirectories.indexOf(f) < 0) {
        fDirectories.add(f);
    }
}
