#include "../JuceLibraryCode/JuceHeader.h"

#if JUCE_INCLUDE_ZLIB_CODE
#include <stdio.h>

#undef OS_CODE
#undef fdopen
#define ZLIB_INTERNAL
#define NO_DUMMY_DECL
#include <juce_core/zip/zlib/adler32.c>
#include <juce_core/zip/zlib/compress.c>
#include <zlib.h>
#undef DO1
#undef DO8

#include <juce_core/zip/zlib/crc32.c>
#include <juce_core/zip/zlib/deflate.c>
#include <juce_core/zip/zlib/inffast.c>
#undef PULLBYTE
#undef LOAD
#undef RESTORE
#undef INITBITS
#undef NEEDBITS
#undef DROPBITS
#undef BYTEBITS

#include <juce_core/zip/zlib/inflate.c>
#include <juce_core/zip/zlib/inftrees.c>
#include <juce_core/zip/zlib/trees.c>
#include <juce_core/zip/zlib/zutil.c>
#undef Byte
#undef fdopen
#undef local
#undef Freq
#undef Code
#undef Dad
#undef Len
#endif
