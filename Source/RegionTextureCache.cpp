#include "RegionTextureCache.h"
#include "RegionToTexture.h"
#include <cassert>
#include <minecraft-file.hpp>

using namespace juce;

void RegionTextureCache::load(PixelARGB *pixels) {
  std::unique_ptr<OpenGLTexture> p(new OpenGLTexture());
  p->loadARGB(pixels, 512, 512);
  fTexture.reset(p.release());

  fLoadTime = Time::getCurrentTime();
}
