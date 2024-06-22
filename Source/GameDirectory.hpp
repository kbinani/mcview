#pragma once

namespace mcview {

class GameDirectory {
  GameDirectory() {}

public:
  juce::File fDirectory;
  juce::String fLevelName;
  juce::Image fIcon;
  std::optional<juce::Time> fLastPlayed;
  Edition fEdition;

  static std::optional<GameDirectory> Open(juce::File directory) {
    GameDirectory gd;
    gd.fLevelName = directory.getFileName();
    gd.fDirectory = directory;
    if (directory.getChildFile("db").exists()) {
      gd.fEdition = Edition::Bedrock;
      if (directory.getChildFile("levelname.txt").existsAsFile()) {
        if (auto stream = directory.getChildFile("levelname.txt").createInputStream(); stream && stream->openedOk()) {
          auto name = stream->readEntireStreamAsString().trim();
          if (name.isNotEmpty()) {
            gd.fLevelName = name;
          }
        }
      }
      if (directory.getChildFile("world_icon.jpeg").existsAsFile()) {
        if (auto stream = directory.getChildFile("world_icon.jpeg").createInputStream(); stream && stream->openedOk()) {
          juce::JPEGImageFormat format;
          gd.fIcon = format.decodeImage(*stream);
        }
      }
      if (directory.getChildFile("level.dat").existsAsFile()) {
        if (auto stream = std::make_shared<mcfile::stream::GzFileInputStream>(PathFromFile(directory.getChildFile("level.dat"))); stream && stream->valid() && stream->seek(8)) {
          if (auto compound = mcfile::nbt::CompoundTag::Read(stream, mcfile::Encoding::LittleEndian); compound) {
            if (auto lastPlayed = compound->int64(u8"LastPlayed"); lastPlayed) {
              gd.fLastPlayed = juce::Time(*lastPlayed * 1000);
            }
          }
        }
      }
      return gd;
    } else if (directory.getChildFile("level.dat").existsAsFile()) {
      gd.fEdition = Edition::Java;
      if (auto stream = std::make_shared<mcfile::stream::GzFileInputStream>(PathFromFile(directory.getChildFile("level.dat"))); stream && stream->valid()) {
        if (auto dat = mcfile::nbt::CompoundTag::Read(stream, mcfile::Encoding::Java); dat) {
          if (auto data = dat->compoundTag(u8"Data"); data) {
            if (auto name = data->string(u8"LevelName"); name) {
              auto n = juce::String::fromUTF8(name->c_str());
              if (n.isNotEmpty()) {
                gd.fLevelName = n;
              }
            }
            if (auto lastPlayed = data->int64(u8"LastPlayed"); lastPlayed) {
              gd.fLastPlayed = juce::Time(*lastPlayed);
            }
          }
        }
      }
      if (directory.getChildFile("icon.png").existsAsFile()) {
        if (auto stream = directory.getChildFile("icon.png").createInputStream(); stream && stream->openedOk()) {
          juce::PNGImageFormat format;
          gd.fIcon = format.decodeImage(*stream);
        }
      }
      return gd;
    } else {
      return std::nullopt;
    }
  }

  void draw(
      juce::Graphics &g,
      juce::Rectangle<int> bounds,
      bool selected,
      bool omitEdition,
      juce::Colour textColorOn,
      juce::Colour textColorOff,
      juce::Colour backgroundColorOn,
      juce::Colour backgroundColorOff) const {
    g.saveState();
    defer {
      g.restoreState();
    };

    g.setColour(selected ? backgroundColorOn : backgroundColorOff);
    g.fillRect(bounds);

    bounds.reduce(2, 2);

    if (fIcon.isValid()) {
      auto targetArea = bounds.removeFromLeft(bounds.getHeight());
      g.saveState();
      g.reduceClipRegion(targetArea);
      g.drawImage(fIcon, targetArea.toFloat(), juce::RectanglePlacement::fillDestination);
      g.restoreState();
      g.setColour(backgroundColorOn);
      g.drawRect(targetArea);
    } else {
      auto targetArea = bounds.removeFromLeft(bounds.getHeight());
      g.setColour(backgroundColorOn);
      g.drawRect(targetArea);
    }

    bounds.removeFromLeft(2);

    int lineHeight = bounds.getHeight() / 3;
    auto levelNameArea = bounds.removeFromTop(lineHeight);
    g.setColour(selected ? textColorOn : textColorOff);
    auto levelName = fLevelName;
    if (!omitEdition) {
      switch (fEdition) {
      case Edition::Java:
        levelName += " (Java)";
        break;
      case Edition::Bedrock:
        levelName += " (Bedrock)";
        break;
      }
    }
    g.drawFittedText(levelName, levelNameArea, juce::Justification::centredLeft, 1);

    auto directoryNameArea = bounds.removeFromTop(lineHeight);
    g.setColour(juce::Colours::grey);
    g.drawFittedText(fDirectory.getFileName(), directoryNameArea, juce::Justification::centredLeft, 1);

    auto lastPlayedArea = bounds;
    if (fLastPlayed) {
      g.drawFittedText(StringFromTime(*fLastPlayed), lastPlayedArea, juce::Justification::centredLeft, 1);
    }
  }

  static bool Compare(GameDirectory const &left, GameDirectory const &right) {
    if (left.fLastPlayed && right.fLastPlayed) {
      if (*left.fLastPlayed == *right.fLastPlayed) {
        return left.fDirectory.getFileName().compare(right.fDirectory.getFileName()) < 0;
      } else {
        return *left.fLastPlayed > *right.fLastPlayed;
      }
    } else if (left.fLastPlayed) {
      return true;
    } else {
      return false;
    }
  }

  static bool HasInterest(juce::File const &file) {
    if (file == juce::File()) {
      return false;
    }
    if (file == DefaultJavaSaveDirectory()) {
      return true;
    } else if (file == DefaultBedrockSaveDirectory()) {
      return true;
    } else if (!file.isDirectory()) {
      return false;
    }
    auto db = file.getChildFile("db");
    if (db.exists() && db.isDirectory()) {
      return true;
    }
    auto dat = file.getChildFile("level.dat");
    if (dat.existsAsFile()) {
      return true;
    }
    return false;
  }

  static juce::String StringFromTime(juce::Time const &t) {
    int year = t.getYear();
    int month = t.getMonth() + 1;
    int day = t.getDayOfMonth();
    int hour = t.getHours();
    int minute = t.getMinutes();
    return juce::String(year) + "/" + juce::String::formatted("%02d", month) + "/" + juce::String::formatted("%02d", day) + " " + juce::String(hour) + ":" + juce::String::formatted("%02d", minute);
  }
};

} // namespace mcview
