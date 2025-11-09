#pragma once

namespace mcview {

inline std::filesystem::path PathFromFile(juce::File const &file) {
#if defined(_WIN32)
  return std::filesystem::path(file.getFullPathName().toWideCharPointer());
#else
  return std::filesystem::path(file.getFullPathName().toStdString());
#endif
}

static juce::File DefaultJavaSaveDirectory() {
  using namespace juce;
#if JUCE_WINDOWS
  return File::getSpecialLocation(File::userApplicationDataDirectory).getChildFile(".minecraft").getChildFile("saves");
#else
  File library = File::getSpecialLocation(File::userHomeDirectory).getParentDirectory().getParentDirectory().getParentDirectory();
  return library.getChildFile("Application Support").getChildFile("minecraft").getChildFile("saves");
#endif
}

static juce::File DefaultBedrockSaveDirectory() {
#if JUCE_WINDOWS
  auto userApplicationDataDirectory = juce::File::getSpecialLocation(juce::File::userApplicationDataDirectory);
  auto users = userApplicationDataDirectory.getChildFile("Minecraft Bedrock").getChildFile("Users");
  if (users.exists() && users.isDirectory()) {
    juce::DirectoryIterator itr(users, false, "*", juce::File::findDirectories);
    juce::String userId;
    juce::String const digits = "0123456789";
    while (itr.next()) {
      auto sub = itr.getFile();
      auto n = sub.getFileName();
      if (n == u8"Shared") {
        continue;
      }
      if (n.startsWith("-")) {
        // Not sure if negative userId exists
        if (n.length() == 1) {
          continue;
        }
        if (!n.substring(1).containsOnly(digits)) {
          continue;
        }
      } else {
        if (!n.containsOnly(digits)) {
          continue;
        }
      }
      auto moj = sub.getChildFile("games").getChildFile("com.mojang");
      if (!moj.exists() || !moj.isDirectory()) {
        continue;
      }
      userId = n;
      break;
    }
    if (userId.isNotEmpty()) {
      return users.getChildFile(userId).getChildFile("games").getChildFile("com.mojang").getChildFile("minecraftWorlds");
    }
  }
  return userApplicationDataDirectory
      .getParentDirectory()
      .getChildFile("Local")
      .getChildFile("Packages")
      .getChildFile("Microsoft.MinecraftUWP_8wekyb3d8bbwe")
      .getChildFile("LocalState")
      .getChildFile("games")
      .getChildFile("com.mojang")
      .getChildFile("minecraftWorlds");
#else
  return File();
#endif
}

static juce::File TempDirectory() {
#if JUCE_WINDOWS
  return juce::File::getSpecialLocation(juce::File::SpecialLocationType::tempDirectory).getChildFile("mcview");
#else
  return juce::File::getSpecialLocation(juce::File::SpecialLocationType::tempDirectory);
#endif
}

static juce::File CacheDirectory() {
  return TempDirectory().getChildFile("cache");
}

static juce::File WorkingDirectory() {
  return TempDirectory().getChildFile("runtime");
}

} // namespace mcview
