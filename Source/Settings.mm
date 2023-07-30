#import <Foundation/Foundation.h>

#include <juce_gui_basics/juce_gui_basics.h>
#include <nlohmann/json.hpp>

#include "Edition.hpp"

#include "Directory.hpp"
#include "LightingType.hpp"
#include "PaletteType.hpp"

#include "Settings.hpp"

using namespace juce;

namespace mcview {

String Settings::Bookmark(File f)
{
    JUCE_AUTORELEASEPOOL {
        URL u(f);
        String s = u.toString(false);
        NSString *ns = [[NSString alloc] initWithUTF8String: s.toRawUTF8()];
        NSURL *nsU = [[NSURL alloc] initWithString: ns];
        NSError *e = nullptr;
        NSData *data = [nsU bookmarkDataWithOptions:NSURLBookmarkCreationWithSecurityScope includingResourceValuesForKeys:nullptr relativeToURL:nullptr error:&e];
        if (e != nullptr) {
            return String();
        }
        NSString *b64 = [data base64EncodedStringWithOptions: 0];
        String b64s = String::fromUTF8([b64 UTF8String]);
        return b64s;
    }
}

File Settings::FromBookmark(String s)
{
    JUCE_AUTORELEASEPOOL {
        NSString *b64 = [[NSString alloc] initWithUTF8String: s.toRawUTF8()];
        NSData *data = [[NSData alloc] initWithBase64EncodedString:b64 options:0];
        NSError *e = nullptr;
        NSURL *nsU = [NSURL URLByResolvingBookmarkData:data options:NSURLBookmarkResolutionWithSecurityScope relativeToURL:nullptr bookmarkDataIsStale:nullptr error:&e];
        if (e != nullptr) {
            return File();
        }
        [nsU startAccessingSecurityScopedResource];
        NSString *ns = [nsU absoluteString];
        String ss = String::fromUTF8([ns UTF8String]);
        URL u(ss);
        return u.getLocalFile();
    }
}

}
