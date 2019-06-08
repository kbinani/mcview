#include "RegionTextureCache.h"
#include "RegionToTexture.h"
#include <cassert>
#include <minecraft-file.hpp>

void RegionTextureCache::loadIfNeeded()
{
    auto region = mcfile::Region::MakeRegion(fFile.toStdString());
    if (!region) return;
    if (fTexture && fHeightmap) return;
    RegionToTexture::Load(*region, [this](PixelARGB *pixels, uint8 *heightMap) {
        if (pixels == nullptr || heightMap == nullptr) {
            if (pixels != nullptr) {
                delete[] pixels;
            }
            if (heightMap != nullptr) {
                delete[] heightMap;
            }
            return;
        }
        
        ScopedPointer<OpenGLTexture> texture = new OpenGLTexture();
        texture->loadARGBFlipped(pixels, 512, 512);
        delete[] pixels;
        
        ScopedPointer<OpenGLTexture> heightmap = new OpenGLTexture();
        heightmap->loadAlpha(heightMap, 512, 512);
        delete[] heightMap;
        
        fTexture = texture;
        fHeightmap = heightmap;
    });
    fLoadTime = Time::getCurrentTime();
}
