/*
  ==============================================================================

   This file is part of the JUCE library.
   Copyright (c) 2017 - ROLI Ltd.

   JUCE is an open source library subject to commercial or open-source
   licensing.

   By using JUCE, you agree to the terms of both the JUCE 5 End-User License
   Agreement and JUCE 5 Privacy Policy (both updated and effective as of the
   27th April 2017).

   End User License Agreement: www.juce.com/juce-5-licence
   Privacy Policy: www.juce.com/juce-5-privacy-policy

   Or: You may also use this code under the terms of the GPL v3 (see
   www.gnu.org/licenses).

   JUCE IS PROVIDED "AS IS" WITHOUT ANY WARRANTY, AND ALL WARRANTIES, WHETHER
   EXPRESSED OR IMPLIED, INCLUDING MERCHANTABILITY AND FITNESS FOR PURPOSE, ARE
   DISCLAIMED.

  ==============================================================================
*/

#include "PNGWriter.h"
#include <cassert>

namespace juce
{

#if JUCE_MSVC
 #pragma warning (push)
 #pragma warning (disable: 4390 4611 4365 4267)
 #ifdef __INTEL_COMPILER
  #pragma warning (disable: 2544 2545)
 #endif
#endif

namespace zlibNamespace
{
#if JUCE_INCLUDE_ZLIB_CODE
  #undef OS_CODE
  #undef fdopen
  #include "juce_core/zip/zlib/zlib.h"
  #undef OS_CODE
#else
  #include JUCE_ZLIB_INCLUDE_PATH
#endif
}

#if ! defined (jmp_buf) || ! defined (longjmp)
 #include <setjmp.h>
#endif

namespace pnglibNamespace
{
  using namespace zlibNamespace;

#if JUCE_INCLUDE_PNGLIB_CODE || ! defined (JUCE_INCLUDE_PNGLIB_CODE)

  #if _MSC_VER != 1310
   using std::calloc; // (causes conflict in VS.NET 2003)
   using std::malloc;
   using std::free;
  #endif

  #if JUCE_CLANG
   #pragma clang diagnostic push
   #pragma clang diagnostic ignored "-Wsign-conversion"
   #if __has_warning("-Wzero-as-null-pointer-constant")
    #pragma clang diagnostic ignored "-Wzero-as-null-pointer-constant"
   #endif
   #if __has_warning("-Wcomma")
    #pragma clang diagnostic ignored "-Wcomma"
   #endif
  #endif

  #undef check
  using std::abs;
  #define NO_DUMMY_DECL
  #define PNGLCONF_H 1

 #if JUCE_ANDROID
  #define PNG_ARM_NEON_SUPPORTED
 #endif

  #define PNG_16BIT_SUPPORTED
  #define PNG_ALIGNED_MEMORY_SUPPORTED
  #define PNG_BENIGN_ERRORS_SUPPORTED
  #define PNG_BENIGN_READ_ERRORS_SUPPORTED
  #define PNG_BUILD_GRAYSCALE_PALETTE_SUPPORTED
  #define PNG_CHECK_FOR_INVALID_INDEX_SUPPORTED
  #define PNG_COLORSPACE_SUPPORTED
  #define PNG_CONSOLE_IO_SUPPORTED
  #define PNG_EASY_ACCESS_SUPPORTED
  #define PNG_FIXED_POINT_SUPPORTED
  #define PNG_FLOATING_ARITHMETIC_SUPPORTED
  #define PNG_FLOATING_POINT_SUPPORTED
  #define PNG_FORMAT_AFIRST_SUPPORTED
  #define PNG_FORMAT_BGR_SUPPORTED
  #define PNG_GAMMA_SUPPORTED
  #define PNG_GET_PALETTE_MAX_SUPPORTED
  #define PNG_HANDLE_AS_UNKNOWN_SUPPORTED
  #define PNG_INCH_CONVERSIONS_SUPPORTED
  #define PNG_INFO_IMAGE_SUPPORTED
  #define PNG_IO_STATE_SUPPORTED
  #define PNG_MNG_FEATURES_SUPPORTED
  #define PNG_POINTER_INDEXING_SUPPORTED
  #define PNG_PROGRESSIVE_READ_SUPPORTED
  #define PNG_READ_16BIT_SUPPORTED
  #define PNG_READ_ALPHA_MODE_SUPPORTED
  #define PNG_READ_ANCILLARY_CHUNKS_SUPPORTED
  #define PNG_READ_BACKGROUND_SUPPORTED
  #define PNG_READ_BGR_SUPPORTED
  #define PNG_READ_CHECK_FOR_INVALID_INDEX_SUPPORTED
  #define PNG_READ_COMPOSITE_NODIV_SUPPORTED
  #define PNG_READ_COMPRESSED_TEXT_SUPPORTED
  #define PNG_READ_EXPAND_16_SUPPORTED
  #define PNG_READ_EXPAND_SUPPORTED
  #define PNG_READ_FILLER_SUPPORTED
  #define PNG_READ_GAMMA_SUPPORTED
  #define PNG_READ_GET_PALETTE_MAX_SUPPORTED
  #define PNG_READ_GRAY_TO_RGB_SUPPORTED
  #define PNG_READ_INTERLACING_SUPPORTED
  #define PNG_READ_INT_FUNCTIONS_SUPPORTED
  #define PNG_READ_INVERT_ALPHA_SUPPORTED
  #define PNG_READ_INVERT_SUPPORTED
  #define PNG_READ_OPT_PLTE_SUPPORTED
  #define PNG_READ_PACKSWAP_SUPPORTED
  #define PNG_READ_PACK_SUPPORTED
  #define PNG_READ_QUANTIZE_SUPPORTED
  #define PNG_READ_RGB_TO_GRAY_SUPPORTED
  #define PNG_READ_SCALE_16_TO_8_SUPPORTED
  #define PNG_READ_SHIFT_SUPPORTED
  #define PNG_READ_STRIP_16_TO_8_SUPPORTED
  #define PNG_READ_STRIP_ALPHA_SUPPORTED
  #define PNG_READ_SUPPORTED
  #define PNG_READ_SWAP_ALPHA_SUPPORTED
  #define PNG_READ_SWAP_SUPPORTED
  #define PNG_READ_TEXT_SUPPORTED
  #define PNG_READ_TRANSFORMS_SUPPORTED
  #define PNG_READ_UNKNOWN_CHUNKS_SUPPORTED
  #define PNG_READ_USER_CHUNKS_SUPPORTED
  #define PNG_READ_USER_TRANSFORM_SUPPORTED
  #define PNG_READ_bKGD_SUPPORTED
  #define PNG_READ_cHRM_SUPPORTED
  #define PNG_READ_gAMA_SUPPORTED
  #define PNG_READ_hIST_SUPPORTED
  #define PNG_READ_iCCP_SUPPORTED
  #define PNG_READ_iTXt_SUPPORTED
  #define PNG_READ_oFFs_SUPPORTED
  #define PNG_READ_pCAL_SUPPORTED
  #define PNG_READ_pHYs_SUPPORTED
  #define PNG_READ_sBIT_SUPPORTED
  #define PNG_READ_sCAL_SUPPORTED
  #define PNG_READ_sPLT_SUPPORTED
  #define PNG_READ_sRGB_SUPPORTED
  #define PNG_READ_tEXt_SUPPORTED
  #define PNG_READ_tIME_SUPPORTED
  #define PNG_READ_tRNS_SUPPORTED
  #define PNG_READ_zTXt_SUPPORTED
  #define PNG_SAVE_INT_32_SUPPORTED
  #define PNG_SAVE_UNKNOWN_CHUNKS_SUPPORTED
  #define PNG_SEQUENTIAL_READ_SUPPORTED
  #define PNG_SET_CHUNK_CACHE_LIMIT_SUPPORTED
  #define PNG_SET_CHUNK_MALLOC_LIMIT_SUPPORTED
  #define PNG_SET_UNKNOWN_CHUNKS_SUPPORTED
  #define PNG_SET_USER_LIMITS_SUPPORTED
  #define PNG_SIMPLIFIED_READ_AFIRST_SUPPORTED
  #define PNG_SIMPLIFIED_READ_BGR_SUPPORTED
  #define PNG_SIMPLIFIED_WRITE_AFIRST_SUPPORTED
  #define PNG_SIMPLIFIED_WRITE_BGR_SUPPORTED
  #define PNG_STDIO_SUPPORTED
  #define PNG_STORE_UNKNOWN_CHUNKS_SUPPORTED
  #define PNG_TEXT_SUPPORTED
  #define PNG_TIME_RFC1123_SUPPORTED
  #define PNG_UNKNOWN_CHUNKS_SUPPORTED
  #define PNG_USER_CHUNKS_SUPPORTED
  #define PNG_USER_LIMITS_SUPPORTED
  #define PNG_USER_TRANSFORM_INFO_SUPPORTED
  #define PNG_USER_TRANSFORM_PTR_SUPPORTED
  #define PNG_WARNINGS_SUPPORTED
  #define PNG_WRITE_16BIT_SUPPORTED
  #define PNG_WRITE_ANCILLARY_CHUNKS_SUPPORTED
  #define PNG_WRITE_BGR_SUPPORTED
  #define PNG_WRITE_CHECK_FOR_INVALID_INDEX_SUPPORTED
  #define PNG_WRITE_COMPRESSED_TEXT_SUPPORTED
  #define PNG_WRITE_CUSTOMIZE_ZTXT_COMPRESSION_SUPPORTED
  #define PNG_WRITE_FILLER_SUPPORTED
  #define PNG_WRITE_FILTER_SUPPORTED
  #define PNG_WRITE_FLUSH_SUPPORTED
  #define PNG_WRITE_GET_PALETTE_MAX_SUPPORTED
  #define PNG_WRITE_INTERLACING_SUPPORTED
  #define PNG_WRITE_INT_FUNCTIONS_SUPPORTED
  #define PNG_WRITE_INVERT_ALPHA_SUPPORTED
  #define PNG_WRITE_INVERT_SUPPORTED
  #define PNG_WRITE_OPTIMIZE_CMF_SUPPORTED
  #define PNG_WRITE_PACKSWAP_SUPPORTED
  #define PNG_WRITE_PACK_SUPPORTED
  #define PNG_WRITE_SHIFT_SUPPORTED
  #define PNG_WRITE_SUPPORTED
  #define PNG_WRITE_SWAP_ALPHA_SUPPORTED
  #define PNG_WRITE_SWAP_SUPPORTED
  #define PNG_WRITE_TEXT_SUPPORTED
  #define PNG_WRITE_TRANSFORMS_SUPPORTED
  #define PNG_WRITE_UNKNOWN_CHUNKS_SUPPORTED
  #define PNG_WRITE_USER_TRANSFORM_SUPPORTED
  #define PNG_WRITE_WEIGHTED_FILTER_SUPPORTED
  #define PNG_WRITE_bKGD_SUPPORTED
  #define PNG_WRITE_cHRM_SUPPORTED
  #define PNG_WRITE_gAMA_SUPPORTED
  #define PNG_WRITE_hIST_SUPPORTED
  #define PNG_WRITE_iCCP_SUPPORTED
  #define PNG_WRITE_iTXt_SUPPORTED
  #define PNG_WRITE_oFFs_SUPPORTED
  #define PNG_WRITE_pCAL_SUPPORTED
  #define PNG_WRITE_pHYs_SUPPORTED
  #define PNG_WRITE_sBIT_SUPPORTED
  #define PNG_WRITE_sCAL_SUPPORTED
  #define PNG_WRITE_sPLT_SUPPORTED
  #define PNG_WRITE_sRGB_SUPPORTED
  #define PNG_WRITE_tEXt_SUPPORTED
  #define PNG_WRITE_tIME_SUPPORTED
  #define PNG_WRITE_tRNS_SUPPORTED
  #define PNG_WRITE_zTXt_SUPPORTED
  #define PNG_bKGD_SUPPORTED
  #define PNG_cHRM_SUPPORTED
  #define PNG_gAMA_SUPPORTED
  #define PNG_hIST_SUPPORTED
  #define PNG_iCCP_SUPPORTED
  #define PNG_iTXt_SUPPORTED
  #define PNG_oFFs_SUPPORTED
  #define PNG_pCAL_SUPPORTED
  #define PNG_pHYs_SUPPORTED
  #define PNG_sBIT_SUPPORTED
  #define PNG_sCAL_SUPPORTED
  #define PNG_sPLT_SUPPORTED
  #define PNG_sRGB_SUPPORTED
  #define PNG_tEXt_SUPPORTED
  #define PNG_tIME_SUPPORTED
  #define PNG_tRNS_SUPPORTED
  #define PNG_zTXt_SUPPORTED

  #define PNG_STRING_COPYRIGHT "";
  #define PNG_STRING_NEWLINE "\n"
  #define PNG_LITERAL_SHARP 0x23
  #define PNG_LITERAL_LEFT_SQUARE_BRACKET 0x5b
  #define PNG_LITERAL_RIGHT_SQUARE_BRACKET 0x5d

  #define PNG_API_RULE 0
  #define PNG_CALLOC_SUPPORTED
  #define PNG_COST_SHIFT 3
  #define PNG_DEFAULT_READ_MACROS 1
  #define PNG_GAMMA_THRESHOLD_FIXED 5000
  #define PNG_IDAT_READ_SIZE PNG_ZBUF_SIZE
  #define PNG_INFLATE_BUF_SIZE 1024
  #define PNG_MAX_GAMMA_8 11
  #define PNG_QUANTIZE_BLUE_BITS 5
  #define PNG_QUANTIZE_GREEN_BITS 5
  #define PNG_QUANTIZE_RED_BITS 5
  #define PNG_TEXT_Z_DEFAULT_COMPRESSION (-1)
  #define PNG_TEXT_Z_DEFAULT_STRATEGY 0
  #define PNG_WEIGHT_SHIFT 8
  #define PNG_ZBUF_SIZE 8192
  #define PNG_Z_DEFAULT_COMPRESSION (-1)
  #define PNG_Z_DEFAULT_NOFILTER_STRATEGY 0
  #define PNG_Z_DEFAULT_STRATEGY 1
  #define PNG_sCAL_PRECISION 5
  #define PNG_sRGB_PROFILE_CHECKS 2

  #define png_debug(a, b)
  #define png_debug1(a, b, c)
  #define png_debug2(a, b, c, d)

  #include "pnglib/png.h"
  #include "pnglib/pngconf.h"

  #define PNG_NO_EXTERN

  #if JUCE_CLANG
   #pragma clang diagnostic pop
  #endif
#else
  extern "C"
  {
    #include <png.h>
    #include <pngconf.h>
  }
#endif
}

#undef max
#undef min
#undef fdopen

#if JUCE_MSVC
 #pragma warning (pop)
#endif

//==============================================================================
namespace PNGHelpers
{
    using namespace pnglibNamespace;

    static void JUCE_CDECL writeDataCallback (png_structp png, png_bytep data, png_size_t length)
    {
        static_cast<OutputStream*> (png_get_io_ptr (png))->write (data, length);
    }
}
    
} // namespace juce

PNGWriter::PNGWriter(int width, int height, OutputStream &stream)
    : fStream(stream)
    , fWidth(width)
    , fHeight(height)
    , fWriteStruct(nullptr)
    , fInfoStruct(nullptr)
    , fRowData(width * 4)
{
    using namespace pnglibNamespace;
    
    auto pngWriteStruct = png_create_write_struct (PNG_LIBPNG_VER_STRING, nullptr, nullptr, nullptr);
    fWriteStruct = pngWriteStruct;
    
    if (pngWriteStruct == nullptr)
        return;
    
    auto pngInfoStruct = png_create_info_struct (pngWriteStruct);
    
    if (pngInfoStruct == nullptr)
    {
        png_destroy_write_struct (&pngWriteStruct, nullptr);
        return;
    }
    fInfoStruct = pngInfoStruct;
    
    png_set_write_fn (pngWriteStruct, &fStream, PNGHelpers::writeDataCallback, nullptr);
    
    png_set_IHDR (pngWriteStruct, pngInfoStruct, (png_uint_32) width, (png_uint_32) height, 8,
                  PNG_COLOR_TYPE_RGB_ALPHA,
                  PNG_INTERLACE_NONE,
                  PNG_COMPRESSION_TYPE_BASE,
                  PNG_FILTER_TYPE_BASE);
    
    png_color_8 sig_bit;
    sig_bit.red   = 8;
    sig_bit.green = 8;
    sig_bit.blue  = 8;
    sig_bit.gray  = 0;
    sig_bit.alpha = 8;
    png_set_sBIT (pngWriteStruct, pngInfoStruct, &sig_bit);
    
    png_write_info (pngWriteStruct, pngInfoStruct);
    
    png_set_shift (pngWriteStruct, &sig_bit);
    png_set_packing (pngWriteStruct);
    
}

PNGWriter::~PNGWriter()
{
    if (!fWriteStruct || !fInfoStruct) {
        return;
    }
    using namespace pnglibNamespace;

    png_structrp pngWriteStruct = (png_structrp)fWriteStruct;
    png_infop pngInfoStruct = (png_infop)fInfoStruct;
    png_write_end (pngWriteStruct, pngInfoStruct);
    png_destroy_write_struct (&pngWriteStruct, &pngInfoStruct);
}

void PNGWriter::writeRow(PixelARGB *row)
{
    using namespace pnglibNamespace;

    uint8* dst = fRowData;
    
    for (int i = fWidth; --i >= 0;) {
        PixelARGB p = row[fWidth - i - 1];
        p.unpremultiply();
        
        *dst++ = p.getRed();
        *dst++ = p.getGreen();
        *dst++ = p.getBlue();
        *dst++ = p.getAlpha();
    }
    
    png_bytep rowPtr = fRowData;
    png_structp pngWriteStruct = (png_structp)fWriteStruct;
    png_write_rows (pngWriteStruct, &rowPtr, 1);
}
