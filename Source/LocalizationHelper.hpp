#pragma once

namespace mcview {

class LocalizationHelper {
public:
  static juce::LocalisedStrings *CurrentLocalisedStrings();

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
