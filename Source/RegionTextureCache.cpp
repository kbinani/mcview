#include "RegionTextureCache.h"
#include "RegionToTexture.h"
#include <cassert>
#include <minecraft-file.hpp>

void RegionTextureCache::load(PixelARGB* pixels)
{
    ScopedPointer<OpenGLTexture> p = new OpenGLTexture();
    p->loadARGBFlipped(pixels, 512, 512);
    fTexture.reset(p.release());

    fLoadTime = Time::getCurrentTime();
}
