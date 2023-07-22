#pragma once

namespace mcview {

struct GLBuffer {
  GLuint vBuffer;
  GLuint iBuffer;

  static GLsizei const kNumPoints = 4;
};

} // namespace mcview
