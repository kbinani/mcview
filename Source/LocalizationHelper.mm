#import <Foundation/Foundation.h>

#include <juce_gui_basics/juce_gui_basics.h>

#include "BinaryData.hpp"
#include "LocalizationHelper.hpp"

using namespace juce;

namespace mcview {

#if JUCE_MAC

LocalisedStrings *LocalizationHelper::CurrentLocalisedStrings() {
  NSString * language = [[NSLocale preferredLanguages] firstObject];
  if ([language isEqualToString: @"ja-JP"]) {
    return Japanese();
  }
  return nullptr;
}

std::vector<juce::String> LocalizationHelper::PreferredLanguages() {
  std::vector<juce::String> ret;
  for (NSString *language in [NSLocale preferredLanguages]) {
    ret.push_back(juce::String::fromUTF8([language UTF8String]));
  }
  return ret;
}

#endif

}
