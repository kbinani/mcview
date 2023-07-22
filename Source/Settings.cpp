#include "Settings.hpp"
#include "SettingsComponent.hpp"

using namespace juce;

static const Identifier kDirectories("directories");
static const Identifier kWaterTranslucent("water_translucent");
static const Identifier kWaterOpticalDensity("water_optical_density");
static const Identifier kBiomeEnabled("biome_enabled");
static const Identifier kBiomeBlend("biome_blend");
static const Identifier kShowPin("show_pin");

static bool GetInt(var &v, Identifier key, int *r, int min, int max) {
  if (!v.hasProperty(key)) {
    return false;
  }
  var vv = v.getProperty(key, var());
  if (!vv.isInt()) {
    return false;
  }
  *r = std::min(std::max((int)vv, min), max);
  return true;
}

static bool GetBool(var &v, Identifier key, bool *r) {
  if (!v.hasProperty(key)) {
    return false;
  }
  var vv = v.getProperty(key, var());
  if (!vv.isBool()) {
    return false;
  }
  *r = (bool)vv;
  return true;
}

static bool GetFloat(var &v, Identifier key, float *r, float min, float max) {
  if (!v.hasProperty(key)) {
    return false;
  }
  var vv = v.getProperty(key, var());
  if (!vv.isDouble()) {
    return false;
  }
  *r = std::min(std::max((float)(double)vv, min), max);
  return true;
}

static bool GetStringArray(var &v, Identifier key, Array<String> &r) {
  r.clear();

  if (!v.hasProperty(key)) {
    return false;
  }
  var dirs = v.getProperty(kDirectories, var());
  if (!dirs.isArray()) {
    return false;
  }
  Array<var> *d = dirs.getArray();
  if (!d) {
    return false;
  }
  for (int i = 0; i < d->size(); i++) {
    var f = (*d)[i];
    if (!f.isString()) {
      continue;
    }
    r.add(f);
  }
  return true;
}

File Settings::ConfigDirectory() {
#if JUCE_WINDOWS
  return File::getSpecialLocation(File::userApplicationDataDirectory).getChildFile(".mcview");
#else
  return File::getSpecialLocation(File::userApplicationDataDirectory).getChildFile("Application Support").getChildFile(".mcview");
#endif
}

File Settings::ConfigFile() {
  return ConfigDirectory().getChildFile("config.json");
}

Settings::Settings()
    : fWaterOpticalDensity(SettingsComponent::kDefaultWaterOpticalDensity), fWaterTranslucent(true), fBiomeEnabled(true), fBiomeBlend(SettingsComponent::kDefaultBiomeBlend), fShowPin(true) {
}

void Settings::load() {
  File configFile = ConfigFile();
  FileInputStream stream(configFile);
  if (!stream.openedOk()) {
    return;
  }
  var v = JSON::parse(stream);

  // directories
  Array<String> dirs;
  GetStringArray(v, kDirectories, dirs);
  fDirectories.clear();
  for (int i = 0; i < dirs.size(); i++) {
#if JUCE_MAC
    File f = FromBookmark(dirs[i]);
#else
    File f(dirs[i]);
#endif
    if (f.exists() && f.isDirectory()) {
      fDirectories.add(f);
    }
  }

  // water_optical_density
  GetFloat(v, kWaterOpticalDensity, &fWaterOpticalDensity,
           SettingsComponent::kMinWaterOpticalDensity,
           SettingsComponent::kMaxWaterOpticalDensity);

  // water_translucent
  GetBool(v, kWaterTranslucent, &fWaterTranslucent);

  // biome_enabled
  GetBool(v, kBiomeEnabled, &fBiomeEnabled);

  // biome_blend
  GetInt(v, kBiomeBlend, &fBiomeBlend,
         SettingsComponent::kMinBiomeBlend,
         SettingsComponent::kMaxBiomeBlend);

  // show_pin
  GetBool(v, kShowPin, &fShowPin);
}

void Settings::save() {
  File dir = ConfigDirectory();
  if (!dir.exists()) {
    dir.createDirectory();
  }
  File configFile = ConfigFile();
  configFile.deleteFile();
  FileOutputStream stream(configFile);
  stream.truncate();
  stream.setPosition(0);

  std::unique_ptr<DynamicObject> obj(new DynamicObject());

  // directories
  Array<var> dirs;
  for (int i = 0; i < fDirectories.size(); i++) {
    File f = fDirectories[i];
#if JUCE_MAC
    String b = Bookmark(f);
    if (b.isEmpty()) {
      continue;
    }
    dirs.add(var(b));
#else
    dirs.add(var(f.getFullPathName()));
#endif
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

  // show_pin
  obj->setProperty(kShowPin, var(fShowPin));

  obj->writeAsJSON(stream, 4, false, 16);
}

void Settings::addDirectory(File f) {
  if (fDirectories.indexOf(f) < 0) {
    fDirectories.add(f);
  }
}

void Settings::removeDirectory(File f) {
  int const idx = fDirectories.indexOf(f);
  if (idx >= 0) {
    fDirectories.remove(idx);
  }
}
