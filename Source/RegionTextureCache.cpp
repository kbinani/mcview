#include "RegionTextureCache.hpp"
#include "RegionToTexture.hpp"
#include <cassert>
#include <minecraft-file.hpp>

using namespace juce;

void RegionTextureCache::load(PixelARGB *pixels) {
  std::unique_ptr<OpenGLTexture> p(new OpenGLTexture());
  p->loadARGB(pixels, 512, 512);
  fTexture.reset(p.release());

  fLoadTime = Time::getCurrentTime();
}
