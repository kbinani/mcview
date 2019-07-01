#if JUCE_WINDOWS
#include <windows.h>
#endif

#include "LocalizationHelper.h"

#if !JUCE_MAC

#if JUCE_WINDOWS

LocalisedStrings *LocalizationHelper::CurrentLocalisedStrings()
{
	LANGID lang = GetSystemDefaultUILanguage();
	if (lang == 0x0411) {
		return Japanese();
	}
	return nullptr;
}

#else

LocalisedStrings* LocalizationHelper::CurrentLocalisedStrings()
{
	return nullptr;
}

#endif

#endif
