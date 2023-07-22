#pragma once

namespace mcview {

inline std::filesystem::path PathFromFile(juce::File const &file) {
#if defined(_WIN32)
  return std::filesystem::path(file.getFullPathName().toWideCharPointer());
#else
  return std::filesystem::path(file.getFullPathName().toStdString());
#endif
}

static juce::File DefaultMinecraftSaveDirectory() {
  using namespace juce;
#if JUCE_WINDOWS
  return File::getSpecialLocation(File::userApplicationDataDirectory).getChildFile(".minecraft").getChildFile("saves");
#else
  File library = File::getSpecialLocation(File::userHomeDirectory).getParentDirectory().getParentDirectory().getParentDirectory();
  return library.getChildFile("Application Support").getChildFile("minecraft").getChildFile("saves");
#endif
}

} // namespace mcview
