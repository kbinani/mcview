#import <Foundation/Foundation.h>
#include "LocalizationHelper.h"

#if JUCE_MAC

LocalisedStrings *LocalizationHelper::CurrentLocalisedStrings()
{
    NSString * language = [[NSLocale preferredLanguages] firstObject];
    if ([language isEqualToString: @"ja-JP"]) {
        return Japanese();
    }
    return nullptr;
}

#endif
