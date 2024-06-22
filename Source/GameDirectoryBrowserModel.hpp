#pragma once

namespace mcview {

class DirectoryBrowserModel : public juce::ListBoxModel {
public:
  class Delegate {
  public:
    virtual ~Delegate() = default;
    virtual void directoryBrowserModelDidSelectDirectory(juce::File directory, juce::String levelName) = 0;
  };

  struct Item {
    juce::File fDirectory;
    juce::String fLevelName;
    juce::Image fIcon;
    std::optional<juce::Time> fLastPlayed;
  };

  DirectoryBrowserModel(Delegate *delegate, juce::File directory, juce::LookAndFeel const &laf)
      : fDirectory(directory), fDelegate(delegate) {

    for (auto const &it : juce::RangedDirectoryIterator(fDirectory, false, "*", juce::File::findDirectories)) {
      juce::File file = it.getFile();
      if (!file.isDirectory()) {
        continue;
      }
      Item item;
      item.fDirectory = file;
      item.fLevelName = file.getFileName();
      if (file.getChildFile("levelname.txt").existsAsFile()) {
        if (auto stream = file.getChildFile("levelname.txt").createInputStream(); stream && stream->openedOk()) {
          item.fLevelName = stream->readEntireStreamAsString().trim();
        }
        if (file.getChildFile("world_icon.jpeg").existsAsFile()) {
          if (auto stream = file.getChildFile("world_icon.jpeg").createInputStream(); stream && stream->openedOk()) {
            juce::JPEGImageFormat format;
            item.fIcon = format.decodeImage(*stream);
          }
        }
        if (file.getChildFile("level.dat").existsAsFile()) {
          if (auto stream = std::make_shared<mcfile::stream::GzFileInputStream>(PathFromFile(file.getChildFile("level.dat"))); stream && stream->valid() && stream->seek(8)) {
            if (auto compound = mcfile::nbt::CompoundTag::Read(stream, mcfile::Encoding::LittleEndian); compound) {
              if (auto lastPlayed = compound->int64(u8"LastPlayed"); lastPlayed) {
                item.fLastPlayed = juce::Time(*lastPlayed * 1000);
              }
            }
          }
        }
      } else if (file.getChildFile("level.dat").existsAsFile()) {
        if (auto stream = std::make_shared<mcfile::stream::GzFileInputStream>(PathFromFile(file.getChildFile("level.dat"))); stream && stream->valid()) {
          if (auto dat = mcfile::nbt::CompoundTag::Read(stream, mcfile::Encoding::Java); dat) {
            if (auto data = dat->compoundTag(u8"Data"); data) {
              if (auto name = data->string(u8"LevelName"); name) {
                item.fLevelName = juce::String::fromUTF8(name->c_str());
              }
              if (auto lastPlayed = data->int64(u8"LastPlayed"); lastPlayed) {
                item.fLastPlayed = juce::Time(*lastPlayed);
              }
            }
          }
        }
        if (file.getChildFile("icon.png").existsAsFile()) {
          if (auto stream = file.getChildFile("icon.png").createInputStream(); stream && stream->openedOk()) {
            juce::PNGImageFormat format;
            item.fIcon = format.decodeImage(*stream);
          }
        }
      }
      fItems.push_back(item);
    }
    std::sort(fItems.begin(), fItems.end(), [](Item const &left, Item const &right) {
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
    });

    applyLookAndFeel(laf);
  }

  int getNumRows() override {
    return fItems.size();
  }

  void paintListBoxItem(int rowNumber, juce::Graphics &g, int width, int height, bool rowIsSelected) override {
    g.saveState();
    defer {
      g.restoreState();
    };
    juce::Rectangle<int> bounds(0, 0, width, height);
    g.setColour(rowIsSelected ? fBackgroundColorOn : fBackgroundColorOff);
    g.fillRect(bounds);

    bounds.reduce(2, 2);

    auto const &item = fItems[rowNumber];
    if (item.fIcon.isValid()) {
      float width = item.fIcon.getWidth() * bounds.getHeight() / item.fIcon.getHeight();
      auto targetArea = bounds.removeFromLeft(bounds.getHeight());
      g.saveState();
      g.reduceClipRegion(targetArea);
      g.drawImage(item.fIcon, targetArea.toFloat(), juce::RectanglePlacement::fillDestination);
      g.restoreState();
      g.setColour(fBackgroundColorOn);
      g.drawRect(targetArea);
    } else {
      auto targetArea = bounds.removeFromLeft(bounds.getHeight());
      g.setColour(fBackgroundColorOn);
      g.drawRect(targetArea);
    }

    bounds.removeFromLeft(2);

    int lineHeight = bounds.getHeight() / 3;
    auto levelNameArea = bounds.removeFromTop(lineHeight);
    g.setColour(rowIsSelected ? fTextColorOn : fTextColorOff);
    g.drawFittedText(item.fLevelName, levelNameArea, juce::Justification::centredLeft, 1);

    auto directoryNameArea = bounds.removeFromTop(lineHeight);
    g.setColour(juce::Colours::grey);
    g.drawFittedText(item.fDirectory.getFileName(), directoryNameArea, juce::Justification::centredLeft, 1);

    auto lastPlayedArea = bounds;
    if (item.fLastPlayed) {
      g.drawFittedText(StringFromTime(*item.fLastPlayed), lastPlayedArea, juce::Justification::centredLeft, 1);
    }
  }

  void applyLookAndFeel(juce::LookAndFeel const &laf) {
    fTextColorOn = laf.findColour(juce::TextButton::ColourIds::textColourOnId);
    fTextColorOff = laf.findColour(juce::TextButton::ColourIds::textColourOffId);
    fBackgroundColorOn = laf.findColour(juce::TextButton::ColourIds::buttonOnColourId);
    fBackgroundColorOff = laf.findColour(juce::TextButton::ColourIds::buttonColourId);
  }

  void listBoxItemDoubleClicked(int row, juce::MouseEvent const &) override {
    auto item = fItems[row];
    fDelegate->directoryBrowserModelDidSelectDirectory(item.fDirectory, item.fLevelName);
  }

  juce::String getTooltipForRow(int row) override {
    return fItems[row].fDirectory.getFullPathName();
  }

  void listBoxItemClicked(int row, juce::MouseEvent const &e) override {
    if (e.mods.isRightButtonDown()) {
      juce::PopupMenu menu;
      juce::String title;
#if JUCE_WINDOWS
      title = TRANS("Open in Explorer");
#else
      title = TRANS("Open in Finder");
#endif
      int const id = 1;
      menu.addItem(id, title, true, false);
      juce::PopupMenu::Options o;
      menu.showMenuAsync(o, [this, id, row](int ret) {
        if (ret != id) {
          return;
        }
        fItems[row].fDirectory.revealToUser();
      });
    }
  }

  static juce::String StringFromTime(juce::Time const &t) {
    int year = t.getYear();
    int month = t.getMonth() + 1;
    int day = t.getDayOfMonth();
    int hour = t.getHours();
    int minute = t.getMinutes();
    return juce::String(year) + "/" + juce::String::formatted("%02d", month) + "/" + juce::String::formatted("%02d", day) + " " + juce::String(hour) + ":" + juce::String::formatted("%02d", minute);
  }

private:
  juce::File const fDirectory;
  std::deque<Item> fItems;
  juce::Colour fTextColorOff;
  juce::Colour fTextColorOn;
  juce::Colour fBackgroundColorOff;
  juce::Colour fBackgroundColorOn;
  Delegate *const fDelegate;

  JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(DirectoryBrowserModel)
};

} // namespace mcview
