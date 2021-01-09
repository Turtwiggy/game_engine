#pragma once

// other library headers
#include <GL/glew.h>

namespace fightingengine {

#define CHECK_OPENGL_ERROR(x)                                                                                          \
  GLenum err_##x = glGetError();                                                                                       \
  if (GL_NO_ERROR != err_##x) {                                                                                        \
    printf("\n ~~ ERROR ~~ %i: %s \n", x, gl_error_to_string(err_##x));                                                \
  } else {                                                                                                             \
    printf("\n No OpenGL error %i \n", x);                                                                             \
  }

// http://bits.stephan-brumme.com/roundUpToNextPowerOfTwo.html
unsigned int
next_power_of_two(unsigned int x);

const char*
gl_error_to_string(GLenum err);

} // namespace fightingengine
