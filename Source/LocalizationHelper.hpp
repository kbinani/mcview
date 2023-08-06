#pragma once

namespace mcview {

class LocalizationHelper {
public:
  static juce::LocalisedStrings *CurrentLocalisedStrings() {
#if JUCE_WINDOWS
    LANGID lang = GetSystemDefaultUILanguage();
    if (lang == 0x0411) {
      return Japanese();
    }
    return nullptr;
#elif JUCE_MAC
    NSString *language = [[NSLocale preferredLanguages] firstObject];
    if ([language isEqualToString:@"ja-JP"]) {
      return Japanese();
    }
    return nullptr;
#else
    return nullptr;
#endif
  }

  static std::vector<juce::String> PreferredLanguages() {
#if JUCE_WINDOWS
    using namespace std;
    vector<juce::String> ret;

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
      juce::String lang(buffer.substr(off, idx).c_str());
      if (lang.isNotEmpty()) {
        ret.push_back(lang);
      }
      off = idx + 1;
    }
    juce::String lang(buffer.substr(off).c_str());
    if (lang.isNotEmpty()) {
      ret.push_back(lang);
    }
    return ret;
#elif JUCE_MAC
    std::vector<juce::String> ret;
    for (NSString *language in [NSLocale preferredLanguages]) {
      ret.push_back(juce::String::fromUTF8([language UTF8String]));
    }
    return ret;
#else
    return {};
#endif
  }

private:
  static juce::LocalisedStrings *LoadLocalisedStrings(char const *data, int size) {
    std::vector<char> d(size + 1);
    std::copy_n(data, size, d.begin());
    juce::String t = juce::String::fromUTF8(d.data());
    return new juce::LocalisedStrings(t, false);
  }

  static juce::LocalisedStrings *Japanese() {
    return LoadLocalisedStrings(BinaryData::japanese_lang, BinaryData::japanese_langSize);
  }

  LocalizationHelper() = delete;
};

} // namespace mcview
