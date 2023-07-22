#pragma once

namespace mcview {

static const juce::Identifier kDirectories("directories");
static const juce::Identifier kWaterTranslucent("water_translucent");
static const juce::Identifier kWaterOpticalDensity("water_optical_density");
static const juce::Identifier kBiomeEnabled("biome_enabled");
static const juce::Identifier kBiomeBlend("biome_blend");
static const juce::Identifier kShowPin("show_pin");

class Settings {
public:
  static float constexpr kDefaultWaterOpticalDensity = 0.02f;
  static float constexpr kMaxWaterOpticalDensity = 0.04f;
  static float constexpr kMinWaterOpticalDensity = 0.0f;

  static int constexpr kDefaultBiomeBlend = 2;
  static int constexpr kMaxBiomeBlend = 7;
  static int constexpr kMinBiomeBlend = 0;

public:
  Settings()
      : fWaterOpticalDensity(kDefaultWaterOpticalDensity), fWaterTranslucent(true), fBiomeEnabled(true), fBiomeBlend(kDefaultBiomeBlend), fShowPin(true) {
  }

  juce::Array<juce::File> directories() const {
    return fDirectories;
  }

  void addDirectory(juce::File f) {
    if (fDirectories.indexOf(f) < 0) {
      fDirectories.add(f);
    }
  }

  void removeDirectory(juce::File f) {
    int const idx = fDirectories.indexOf(f);
    if (idx >= 0) {
      fDirectories.remove(idx);
    }
  }

  void load() {
    juce::File configFile = ConfigFile();
    juce::FileInputStream stream(configFile);
    if (!stream.openedOk()) {
      return;
    }
    juce::var v = juce::JSON::parse(stream);

    // directories
    juce::Array<juce::String> dirs;
    GetStringArray(v, kDirectories, dirs);
    fDirectories.clear();
    for (int i = 0; i < dirs.size(); i++) {
#if JUCE_MAC
      File f = FromBookmark(dirs[i]);
#else
      juce::File f(dirs[i]);
#endif
      if (f.exists() && f.isDirectory()) {
        fDirectories.add(f);
      }
    }

    // water_optical_density
    GetFloat(v, kWaterOpticalDensity, &fWaterOpticalDensity,
             kMinWaterOpticalDensity,
             kMaxWaterOpticalDensity);

    // water_translucent
    GetBool(v, kWaterTranslucent, &fWaterTranslucent);

    // biome_enabled
    GetBool(v, kBiomeEnabled, &fBiomeEnabled);

    // biome_blend
    GetInt(v, kBiomeBlend, &fBiomeBlend,
           kMinBiomeBlend,
           kMaxBiomeBlend);

    // show_pin
    GetBool(v, kShowPin, &fShowPin);
  }

  void save() {
    juce::File dir = ConfigDirectory();
    if (!dir.exists()) {
      dir.createDirectory();
    }
    juce::File configFile = ConfigFile();
    configFile.deleteFile();
    juce::FileOutputStream stream(configFile);
    stream.truncate();
    stream.setPosition(0);

    std::unique_ptr<juce::DynamicObject> obj(new juce::DynamicObject());

    // directories
    juce::Array<juce::var> dirs;
    for (int i = 0; i < fDirectories.size(); i++) {
      juce::File f = fDirectories[i];
#if JUCE_MAC
      juce::String b = Bookmark(f);
      if (b.isEmpty()) {
        continue;
      }
      dirs.add(juce::var(b));
#else
      dirs.add(juce::var(f.getFullPathName()));
#endif
    }
    obj->setProperty(kDirectories, juce::var(dirs));

    // water_optical_density
    obj->setProperty(kWaterOpticalDensity, juce::var(fWaterOpticalDensity));

    // water_translucent
    obj->setProperty(kWaterTranslucent, juce::var(fWaterTranslucent));

    // biome_enabled
    obj->setProperty(kBiomeEnabled, juce::var(fBiomeEnabled));

    // biome_blend
    obj->setProperty(kBiomeBlend, juce::var(fBiomeBlend));

    // show_pin
    obj->setProperty(kShowPin, juce::var(fShowPin));

    obj->writeAsJSON(stream, 4, false, 16);
  }

  static juce::File ConfigDirectory() {
#if JUCE_WINDOWS
    return juce::File::getSpecialLocation(juce::File::userApplicationDataDirectory).getChildFile(".mcview");
#else
    return juce::File::getSpecialLocation(juce::File::userApplicationDataDirectory).getChildFile("Application Support").getChildFile(".mcview");
#endif
  }

public:
  float fWaterOpticalDensity;
  bool fWaterTranslucent = true;
  bool fBiomeEnabled = true;
  int fBiomeBlend;
  bool fShowPin = true;

private:
  static juce::File ConfigFile() {
    return ConfigDirectory().getChildFile("config.json");
  }
  static bool GetInt(juce::var &v, juce::Identifier key, int *r, int min, int max) {
    if (!v.hasProperty(key)) {
      return false;
    }
    juce::var vv = v.getProperty(key, juce::var());
    if (!vv.isInt()) {
      return false;
    }
    *r = std::min(std::max((int)vv, min), max);
    return true;
  }

  static bool GetBool(juce::var &v, juce::Identifier key, bool *r) {
    if (!v.hasProperty(key)) {
      return false;
    }
    juce::var vv = v.getProperty(key, juce::var());
    if (!vv.isBool()) {
      return false;
    }
    *r = (bool)vv;
    return true;
  }

  static bool GetFloat(juce::var &v, juce::Identifier key, float *r, float min, float max) {
    if (!v.hasProperty(key)) {
      return false;
    }
    juce::var vv = v.getProperty(key, juce::var());
    if (!vv.isDouble()) {
      return false;
    }
    *r = std::min(std::max((float)(double)vv, min), max);
    return true;
  }

  static bool GetStringArray(juce::var &v, juce::Identifier key, juce::Array<juce::String> &r) {
    r.clear();

    if (!v.hasProperty(key)) {
      return false;
    }
    juce::var dirs = v.getProperty(kDirectories, juce::var());
    if (!dirs.isArray()) {
      return false;
    }
    juce::Array<juce::var> *d = dirs.getArray();
    if (!d) {
      return false;
    }
    for (int i = 0; i < d->size(); i++) {
      juce::var f = (*d)[i];
      if (!f.isString()) {
        continue;
      }
      r.add(f);
    }
    return true;
  }

#if JUCE_MAC
  static juce::String Bookmark(juce::File f);
  static juce::File FromBookmark(juce::String s);
#endif

private:
  juce::Array<juce::File> fDirectories;
};

} // namespace mcview
