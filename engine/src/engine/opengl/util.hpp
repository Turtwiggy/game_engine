#pragma once

// other library headers
#include <GL/glew.h>

#ifdef _DEBUG
#include "thirdparty/magic_enum.hpp"
#endif

namespace fightingengine {

// http://bits.stephan-brumme.com/roundUpToNextPowerOfTwo.html
unsigned int
next_power_of_two(unsigned int x);

const char*
gl_error_to_string(GLenum err);

#ifdef _DEBUG

#define CHECK_OPENGL_ERROR(x)                                                                                          \
  GLenum err_##x = glGetError();                                                                                       \
  if (GL_NO_ERROR != err_##x) {                                                                                        \
    printf("\n ~~ ERROR ~~ %i: %s \n", x, gl_error_to_string(err_##x));                                                \
  }

#endif

} // namespace fightingengine
