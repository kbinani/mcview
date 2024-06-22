#pragma once

namespace mcview {

class GameDirectory {
  GameDirectory() {}

public:
  juce::File fDirectory;
  juce::AttributedString fLevelName;
  juce::Image fIcon;
  std::optional<juce::Time> fLastPlayed;
  Edition fEdition;

  static std::optional<GameDirectory> Open(juce::File directory) {
    GameDirectory gd;
    gd.fLevelName.append(directory.getFileName(), juce::Colours::white);
    gd.fDirectory = directory;
    if (directory.getChildFile("db").exists()) {
      gd.fEdition = Edition::Bedrock;
      if (directory.getChildFile("levelname.txt").existsAsFile()) {
        if (auto stream = directory.getChildFile("levelname.txt").createInputStream(); stream && stream->openedOk()) {
          auto name = stream->readEntireStreamAsString().trim();
          if (name.isNotEmpty()) {
            gd.fLevelName = MakeAttributedString(name);
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
                gd.fLevelName = MakeAttributedString(n);
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
    juce::AttributedString levelName;
    levelName.append(fLevelName);
    if (!omitEdition) {
      switch (fEdition) {
      case Edition::Java:
        levelName.append(" (Java)", juce::Colours::white);
        break;
      case Edition::Bedrock:
        levelName.append(" (Bedrock)", juce::Colours::white);
        break;
      }
    }
    levelName.setJustification(juce::Justification::centredLeft);
    auto text = levelName.getText();
    float width = g.getCurrentFont().getStringWidthFloat(levelName.getText());
    g.saveState();
    g.addTransform(juce::AffineTransform::translation(levelNameArea.getX(), levelNameArea.getY()));
    if (width > 0) {
      float scale = std::min<float>(1, levelNameArea.getWidth() / width);
      g.addTransform(juce::AffineTransform().scaled(scale, 1));
    }
    levelName.draw(g, juce::Rectangle<float>(0, 0, std::numeric_limits<float>::max() * 0.5f, levelNameArea.getHeight()));
    g.restoreState();

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

  static juce::AttributedString MakeAttributedString(juce::String const &s) {
    juce::AttributedString r;
    int offset = 0;
    juce::Colour color = juce::Colours::white;
    while (true) {
      int index = s.indexOf(offset, juce::String(u8"\u00a7"));
      if (index < 0) {
        break;
      }
      r.append(s.substring(offset, index), color);
      auto op = s.substring(index + 1, index + 2);
      offset = index + 2;
      if (op == "0") {
        color = juce::Colour(0x00, 0x00, 0x00);
      } else if (op == "1") {
        color = juce::Colour(0x00, 0x00, 0xaa);
      } else if (op == "2") {
        color = juce::Colour(0x00, 0xAA, 0x00);
      } else if (op == "3") {
        color = juce::Colour(0x00, 0xAA, 0xAA);
      } else if (op == "4") {
        color = juce::Colour(0xAA, 0x00, 0x00);
      } else if (op == "5") {
        color = juce::Colour(0xAA, 0x00, 0xAA);
      } else if (op == "6") {
        color = juce::Colour(0xFF, 0xAA, 0x00);
      } else if (op == "7") {
        color = juce::Colour(0xAA, 0xAA, 0xAA);
      } else if (op == "8") {
        color = juce::Colour(0x55, 0x55, 0x55);
      } else if (op == "9") {
        color = juce::Colour(0x55, 0x55, 0xFF);
      } else if (op == "a") {
        color = juce::Colour(0x55, 0xFF, 0x55);
      } else if (op == "b") {
        color = juce::Colour(0x55, 0xFF, 0xFF);
      } else if (op == "c") {
        color = juce::Colour(0xFF, 0x55, 0x55);
      } else if (op == "d") {
        color = juce::Colour(0xFF, 0x55, 0xFF);
      } else if (op == "e") {
        color = juce::Colour(0xFF, 0xFF, 0x55);
      } else if (op == "f") {
        color = juce::Colour(0xFF, 0xFF, 0xFF);
      } else if (op == "g") {
        color = juce::Colour(221, 214, 5);
      } else if (op == "h") {
        color = juce::Colour(227, 212, 209);
      } else if (op == "i") {
        color = juce::Colour(206, 202, 202);
      } else if (op == "j") {
        color = juce::Colour(68, 58, 59);
      } else if (op == "m") {
        color = juce::Colour(151, 22, 7);
      } else if (op == "n") {
        color = juce::Colour(180, 104, 77);
      } else if (op == "p") {
        color = juce::Colour(222, 177, 45);
      } else if (op == "q") {
        color = juce::Colour(17, 160, 54);
      } else if (op == "s") {
        color = juce::Colour(44, 186, 168);
      } else if (op == "t") {
        color = juce::Colour(33, 73, 123);
      } else if (op == "u") {
        color = juce::Colour(154, 92, 198);
      } else if (op == "r") {
        color = juce::Colours::white;
      }
    }
    if (offset < s.length()) {
      r.append(s.substring(offset), color);
    }
    return r;
  }
};

} // namespace mcview
