#pragma once

namespace mcview {

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
      : fWaterOpticalDensity(kDefaultWaterOpticalDensity),
        fWaterTranslucent(true),
        fBiomeEnabled(true),
        fBiomeBlend(kDefaultBiomeBlend),
        fShowPin(true),
        fPaletteType(PaletteType::mcview),
        fLightingType(LightingType::topLeft) {
  }

  juce::Array<Directory> directories() const {
    return fDirectories;
  }

  void addDirectory(Directory d) {
    for (int i = 0; i < fDirectories.size(); i++) {
      if (fDirectories[i].fDirectory == d.fDirectory) {
        return;
      }
    }
    fDirectories.add(d);
  }

  void removeDirectory(juce::File f) {
    for (int i = 0; i < fDirectories.size(); i++) {
      if (fDirectories[i].fDirectory == f) {
        fDirectories.remove(i);
        return;
      }
    }
  }

  void load() {
    using namespace nlohmann;
    juce::File configFile = ConfigFile();
    juce::FileInputStream stream(configFile);
    if (!stream.openedOk()) {
      return;
    }
    juce::String data = stream.readString();
    std::string str(data.toRawUTF8());
    json obj = json::parse(str, nullptr, false);
    if (obj.is_null()) {
      return;
    }
    if (auto dirs = obj.find("directories"); dirs != obj.end() && dirs->is_array()) {
      for (auto const &dir : *dirs) {
        juce::String path;
        Edition edition = Edition::Java;
        if (dir.is_string()) {
          auto s = dir.get<std::string>();
          path = juce::String::fromUTF8(s.c_str(), s.size());
        } else if (dir.is_object()) {
          if (auto p = dir.find("path"); p != dir.end() && p->is_string()) {
            auto s = p->get<std::string>();
            path = juce::String::fromUTF8(s.c_str(), s.size());
            if (auto e = dir.find("edition"); e != dir.end() && e->is_string()) {
              auto es = e->get<std::string>();
              if (es == "bedrock") {
                edition = Edition::Bedrock;
              }
            }
          } else {
            continue;
          }
        }
#if JUCE_MAC
        juce::File f = FromBookmark(path);
#else
        juce::File f(path);
#endif
        if (f.exists() && f.isDirectory()) {
          Directory d;
          d.fDirectory = f;
          d.fEdition = edition;
          fDirectories.add(d);
        }
      }
    }
    if (auto v = obj.find("water_optical_density"); v != obj.end() && v->is_number()) {
      fWaterOpticalDensity = std::clamp(v->get<float>(), kMinWaterOpticalDensity, kMaxWaterOpticalDensity);
    }
    if (auto v = obj.find("water_translucent"); v != obj.end() && v->is_boolean()) {
      fWaterTranslucent = v->get<bool>();
    }
    if (auto v = obj.find("biome_enabled"); v != obj.end() && v->is_boolean()) {
      fBiomeEnabled = v->get<bool>();
    }
    if (auto v = obj.find("biome_blend"); v != obj.end() && v->is_number_integer()) {
      fBiomeBlend = std::clamp(v->get<int>(), kMinBiomeBlend, kMaxBiomeBlend);
    }
    if (auto v = obj.find("show_pin"); v != obj.end() && v->is_boolean()) {
      fShowPin = v->get<bool>();
    }
    if (auto v = obj.find("palette"); v != obj.end() && v->is_string()) {
      auto s = v->get<std::string>();
      if (s == "java") {
        fPaletteType = PaletteType::java;
      } else if (s == "bedrock") {
        fPaletteType = PaletteType::bedrock;
      } else {
        fPaletteType = PaletteType::mcview;
      }
    }
    if (auto v = obj.find("lighting_type"); v != obj.end() && v->is_string()) {
      auto s = v->get<std::string>();
      if (s == "top") {
        fLightingType = LightingType::top;
      } else {
        fLightingType = LightingType::topLeft;
      }
    }
  }

  /*
  {
    "directories": [
      // "C:\\Users\\kbinani\\AppData\\Roaming\\.minecraft\\saves\\test",
      {
        "path": "C:\\Users\\kbinani\\AppData\\Roaming\\.minecraft\\saves\\test",
        "edition": "java"
      }
    ],
    "water_optical_density": 0.03440860286355019,
    "water_translucent": true,
    "biome_enabled": true,
    "biome_blend": 7,
    "show_pin": true,
    "palette": "java",
    "lighting_type": "top"
  }
   */

  static std::string UTF8StringFromJuceString(juce::String const &s) {
    return std::string(s.toRawUTF8());
  }

  void save() {
    using namespace nlohmann;
    juce::File dir = ConfigDirectory();
    if (!dir.exists()) {
      dir.createDirectory();
    }
    juce::File configFile = ConfigFile();
    json obj;
    json dirs = json::array();
    for (int i = 0; i < fDirectories.size(); i++) {
      json item;
      Directory dir = fDirectories[i];
#if JUCE_MAC
      juce::String b = Bookmark(dir.fDirectory);
      if (b.isEmpty()) {
        continue;
      }
      item["path"] = UTF8StringFromJuceString(b);
#else
      std::string path;
      item["path"] = UTF8StringFromJuceString(dir.fDirectory.getFullPathName());
#endif
      switch (dir.fEdition) {
      case Edition::Bedrock:
        item["edition"] = "bedrock";
        break;
      case Edition::Java:
        item["edition"] = "java";
        break;
      }
      dirs.push_back(item);
    }
    obj["directories"] = dirs;
    obj["water_optical_density"] = fWaterOpticalDensity;
    obj["water_translucent"] = fWaterTranslucent;
    obj["biome_enabled"] = fBiomeEnabled;
    obj["biome_blend"] = fBiomeBlend;
    obj["show_pin"] = fShowPin;
    {
      std::string s = "mcview";
      switch (fPaletteType) {
      case PaletteType::java:
        s = "java";
        break;
      case PaletteType::bedrock:
        s = "bedrock";
        break;
      case PaletteType::mcview:
      default:
        s = "mcview";
        break;
      }
      obj["palette"] = s;
    }
    {
      std::string s = "top-left";
      switch (fLightingType) {
      case LightingType::top:
        s = "top";
        break;
      case LightingType::topLeft:
      default:
        s = "top-left";
        break;
      }
      obj["lighting_type"] = s;
    }
    configFile.deleteFile();
    juce::FileOutputStream stream(configFile);
    stream.truncate();
    stream.setPosition(0);
    std::string data = obj.dump(2);
    stream.write(data.c_str(), data.size());
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
  PaletteType fPaletteType = PaletteType::mcview;
  LightingType fLightingType = LightingType::topLeft;

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

#if JUCE_MAC
  static juce::String Bookmark(juce::File f);
  static juce::File FromBookmark(juce::String s);
#endif

private:
  juce::Array<Directory> fDirectories;
};

} // namespace mcview
