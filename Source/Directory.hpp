#pragma once

namespace mcview {

struct Directory {
  juce::File fDirectory;
  Edition fEdition;

  bool operator==(Directory const &other) const {
    return fDirectory == other.fDirectory && fEdition == other.fEdition;
  }

  static std::optional<Directory> Make(juce::File const &file) {
    if (file == juce::File()) {
      return std::nullopt;
    }
    if (file == DefaultJavaSaveDirectory()) {
      Directory d;
      d.fDirectory = DefaultJavaSaveDirectory();
      d.fEdition = Edition::Java;
      return d;
    } else if (file == DefaultBedrockSaveDirectory()) {
      Directory d;
      d.fDirectory = DefaultBedrockSaveDirectory();
      d.fEdition = Edition::Bedrock;
      return d;
    } else if (!file.isDirectory()) {
      // mcworld
      return std::nullopt;
    }
    auto db = file.getChildFile("db");
    if (db.exists() && db.isDirectory()) {
      Directory d;
      d.fDirectory = file;
      d.fEdition = Edition::Bedrock;
      return d;
    }
    auto dat = file.getChildFile("level.dat");
    if (dat.existsAsFile()) {
      Directory d;
      d.fDirectory = file;
      d.fEdition = Edition::Java;
      return d;
    }
    return std::nullopt;
  }
};

} // namespace mcview
