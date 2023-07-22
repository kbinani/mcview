#include <juce_gui_basics/juce_gui_basics.h>

#include "BinaryData.hpp"

#include "LocalizationHelper.hpp"

#if JUCE_WINDOWS
#include <windows.h>
#endif

using namespace juce;

namespace mcview {

#if !JUCE_MAC

#if JUCE_WINDOWS

LocalisedStrings *LocalizationHelper::CurrentLocalisedStrings() {
  LANGID lang = GetSystemDefaultUILanguage();
  if (lang == 0x0411) {
    return Japanese();
  }
  return nullptr;
}

std::vector<String> LocalizationHelper::PreferredLanguages() {
  using namespace std;
  vector<String> ret;

  ULONG num = 0;
  ULONG size = 0;
  if (!GetSystemPreferredUILanguages(MUI_LANGUAGE_NAME, &num, nullptr, &size)) {
    return ret;
  }
  wstring buffer;
  buffer.resize(size, (WCHAR)0);
  num = 0;
  size = buffer.size();
  if (!GetSystemPreferredUILanguages(MUI_LANGUAGE_NAME, &num, buffer.data(), &size)) {
    return ret;
  }
  int off = 0;
  while (off < buffer.size()) {
    size_t idx = buffer.find(L'\0', off);
    if (idx == wstring::npos) {
      break;
    }
    String lang(buffer.substr(off, idx).c_str());
    if (lang.isNotEmpty()) {
      ret.push_back(lang);
    }
    off = idx + 1;
  }
  String lang(buffer.substr(off).c_str());
  if (lang.isNotEmpty()) {
    ret.push_back(lang);
  }
  return ret;
}
#else

LocalisedStrings *LocalizationHelper::CurrentLocalisedStrings() {
  return nullptr;
}

std::vector<String> LocalizationHelper::PreferredLanguages() {
  return {};
}
#endif

#endif

} // namespace mcview
