#pragma once

namespace mcview {

struct LookAt {
  float fX;
  float fZ;
  float fBlocksPerPixel;

  LookAt() : fX(0), fZ(0), fBlocksPerPixel(5) {}
  LookAt(float x, float z, float blocksPerPixel) : fX(x), fZ(z), fBlocksPerPixel(blocksPerPixel) {}
};

} // namespace mcview
