#pragma once

namespace mcview {

static const juce::Identifier kPins("pins");

class WorldData {
public:
  void save(juce::File path) const {
    juce::var pins = juce::var(juce::Array<juce::var>());
    for (int i = 0; i < fPins.size(); i++) {
      pins.append(fPins[i]->toVar());
    }
    std::unique_ptr<juce::DynamicObject> obj(new juce::DynamicObject());
    obj->setProperty(kPins, pins);

    path.deleteFile();
    juce::FileOutputStream stream(path);
    stream.truncate();
    stream.setPosition(0);
    obj->writeAsJSON(stream, juce::JSON::FormatOptions().withSpacing(juce::JSON::Spacing::singleLine).withIndentLevel(4).withMaxDecimalPlaces(16));
  }

  static WorldData Load(juce::File path) {
    juce::FileInputStream stream(path);
    if (!stream.openedOk()) {
      return WorldData();
    }
    WorldData data;
    juce::var v = juce::JSON::parse(stream);
    juce::var pins = v.getProperty(kPins, juce::Array<juce::var>());
    if (pins.isArray()) {
      for (int i = 0; i < pins.size(); i++) {
        juce::var pin = pins[i];
        std::shared_ptr<Pin> p = std::make_shared<Pin>();
        if (!Pin::Parse(pin, *p)) {
          continue;
        }
        data.fPins.push_back(p);
      }
    }
    return data;
  }

  static juce::File WorldDataPath(juce::File worldDirectory) {
    juce::File dir = Settings::ConfigDirectory();
    juce::int64 id = worldDirectory.getFullPathName().hashCode64();
    return dir.getChildFile(juce::String(id) + ".json");
  }

public:
  std::vector<std::shared_ptr<Pin>> fPins;
};

} // namespace mcview
