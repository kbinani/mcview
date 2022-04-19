#pragma once

#include <juce_gui_extra/juce_gui_extra.h>

inline std::filesystem::path PathFromFile(juce::File const& file) {
#if defined(_WIN32)
  return std::filesystem::path(file.getFullPathName().toWideCharPointer());
#else
  return std::filesystem::path(file.getFullPathName().toStdString());
#endif
}
