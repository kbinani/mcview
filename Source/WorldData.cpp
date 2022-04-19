#include "WorldData.h"
#include "Settings.h"
#include "Pin.h"

using namespace juce;

static const Identifier kPins("pins");

void WorldData::save(File path) const
{
    var pins = var(Array<var>());
    for (int i = 0; i < fPins.size(); i++) {
        pins.append(fPins[i]->toVar());
    }
    std::unique_ptr<DynamicObject> obj(new DynamicObject());
    obj->setProperty(kPins, pins);

    path.deleteFile();
    FileOutputStream stream(path);
    stream.truncate();
    stream.setPosition(0);
    obj->writeAsJSON(stream, 4, false, 16);
}

WorldData WorldData::Load(File path)
{
    FileInputStream stream(path);
    if (!stream.openedOk()) {
        return WorldData();
    }
    WorldData data;
    var v = JSON::parse(stream);
    var pins = v.getProperty(kPins, Array<var>());
    if (pins.isArray()) {
        for (int i = 0; i < pins.size(); i++) {
            var pin = pins[i];
            std::shared_ptr<Pin> p = std::make_shared<Pin>();
            if (!Pin::Parse(pin, *p)) {
                continue;
            }
            data.fPins.push_back(p);
        }
    }
    return data;
}

File WorldData::WorldDataPath(File worldDirectory)
{
    File dir = Settings::ConfigDirectory();
    int64 id = worldDirectory.getFullPathName().hashCode64();
    return dir.getChildFile(String(id) + ".json");
}
