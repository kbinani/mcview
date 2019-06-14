#include "RegionTextureCache.h"
#include "RegionToTexture.h"
#include <cassert>
#include <minecraft-file.hpp>

void RegionTextureCache::load(PixelARGB* pixels, uint8 * heightmap)
{
    ScopedPointer<OpenGLTexture> p = new OpenGLTexture();
    p->loadARGBFlipped(pixels, 512, 512);
    fTexture.reset(p.release());

    ScopedPointer<OpenGLTexture> h = new OpenGLTexture();
    h->loadAlpha(heightmap, 512, 512);
    fHeightmap.reset(h.release());
    
    fLoadTime = Time::getCurrentTime();
}
