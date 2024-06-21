#pragma once

namespace mcview {

class DirectoryBrowserModel : public juce::ListBoxModel {
public:
  class Delegate {
  public:
    virtual ~Delegate() = default;
    virtual void directoryBrowserModelDidSelectDirectory(juce::File directory) = 0;
  };

  struct Item {
    juce::File fDirectory;
    juce::String fLevelName;
  };

  DirectoryBrowserModel(Delegate *delegate, juce::File directory, juce::Colour dotMarkerColor, juce::LookAndFeel const &laf)
      : fDirectory(directory), fDelegate(delegate), fDotMarkerColor(dotMarkerColor) {

    for (auto const &it : juce::RangedDirectoryIterator(fDirectory, false, "*", juce::File::findDirectories)) {
      juce::File file = it.getFile();
      if (!file.isDirectory()) {
        continue;
      }
      Item item;
      item.fDirectory = file;
      item.fLevelName = file.getFileName();
      auto levelnametxt = file.getChildFile("levelname.txt");
      auto leveldat = file.getChildFile("level.dat");
      if (levelnametxt.existsAsFile()) {
        juce::FileInputStream stream(levelnametxt);
        if (stream.openedOk()) {
          item.fLevelName = stream.readEntireStreamAsString().trim();
        }
      } else if (leveldat.existsAsFile()) {
        if (auto stream = std::make_shared<mcfile::stream::GzFileInputStream>(PathFromFile(leveldat)); stream && stream->valid()) {
          if (auto dat = mcfile::nbt::CompoundTag::Read(stream, mcfile::Encoding::Java); dat) {
            if (auto data = dat->compoundTag(u8"Data"); data) {
              if (auto name = data->string(u8"LevelName"); name) {
                item.fLevelName = juce::String::fromUTF8(name->c_str());
              }
            }
          }
        }
      }
      fItems.push_back(item);
    }
    std::sort(fItems.begin(), fItems.end(), [](Item const &left, Item const &right) {
      return left.fDirectory.getFileName().compare(right.fDirectory.getFileName()) < 0;
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

    int margin = 10;
    g.setColour(rowIsSelected ? fBackgroundColorOn : fBackgroundColorOff);
    g.fillRect(0, 0, width, height);

    int radius = (height - 2 * margin) / 2;
    float cx = margin + radius;
    float cy = height * 0.5f;
    int actualRadius = height * 0.5f / 2;
    g.setColour(fDotMarkerColor);
    g.fillEllipse(cx - actualRadius, cy - actualRadius, actualRadius * 2, actualRadius * 2);

    juce::String name;
    auto const &item = fItems[rowNumber];
    if (item.fLevelName.isNotEmpty()) {
      if (item.fLevelName == item.fDirectory.getFileName()) {
        name = item.fLevelName;
      } else {
        name = item.fLevelName + juce::String::fromUTF8(u8"　[") + item.fDirectory.getFileName() + "]";
      }
    } else {
      name = item.fDirectory.getFileName();
    }
    g.setColour(rowIsSelected ? fTextColorOn : fTextColorOff);
    g.drawFittedText(name, margin + radius * 2 + margin, 0, width - 3 * margin - radius * 2, height, juce::Justification::centredLeft, 1);
  }

  void applyLookAndFeel(juce::LookAndFeel const &laf) {
    fTextColorOn = laf.findColour(juce::TextButton::ColourIds::textColourOnId);
    fTextColorOff = laf.findColour(juce::TextButton::ColourIds::textColourOffId);
    fBackgroundColorOn = laf.findColour(juce::TextButton::ColourIds::buttonOnColourId);
    fBackgroundColorOff = laf.findColour(juce::TextButton::ColourIds::buttonColourId);
  }

  void listBoxItemDoubleClicked(int row, juce::MouseEvent const &) override {
    auto item = fItems[row];
    fDelegate->directoryBrowserModelDidSelectDirectory(item.fDirectory);
  }

private:
  juce::File const fDirectory;
  std::deque<Item> fItems;
  juce::Colour fTextColorOff;
  juce::Colour fTextColorOn;
  juce::Colour fBackgroundColorOff;
  juce::Colour fBackgroundColorOn;
  Delegate *const fDelegate;
  juce::Colour fDotMarkerColor;

  JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(DirectoryBrowserModel)
};

} // namespace mcview
