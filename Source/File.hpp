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
  using namespace juce;
#if JUCE_WINDOWS
  return File::getSpecialLocation(File::windowsLocalAppData).getChildFile("Packages").getChildFile("Microsoft.MinecraftUWP_8wekyb3d8bbwe").getChildFile("LocalState").getChildFile("games").getChildFile("com.mojang").getChildFile("minecraftWorlds");
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
