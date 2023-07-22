#import <Foundation/Foundation.h>
#include "LocalizationHelper.h"

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
    return {};
}

#endif

}
