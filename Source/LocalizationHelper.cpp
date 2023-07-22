#include <juce_gui_basics/juce_gui_basics.h>

#include "BinaryData.h"

#include "LocalizationHelper.hpp"

#if JUCE_WINDOWS
#include <windows.h>
#endif

using namespace juce;

#if !JUCE_MAC

#if JUCE_WINDOWS

LocalisedStrings *LocalizationHelper::CurrentLocalisedStrings() {
  LANGID lang = GetSystemDefaultUILanguage();
  if (lang == 0x0411) {
    return Japanese();
  }
  return nullptr;
}

#else

LocalisedStrings *LocalizationHelper::CurrentLocalisedStrings() {
  return nullptr;
}

#endif

#endif
