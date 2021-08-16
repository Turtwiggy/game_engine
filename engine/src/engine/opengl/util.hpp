#pragma once

// c lib headers
#include <stdio.h>

// other library headers
#include <GL/glew.h>
namespace fightingengine {

// http://bits.stephan-brumme.com/roundUpToNextPowerOfTwo.html
unsigned int
next_power_of_two(unsigned int x);

const char*
gl_error_to_string(GLenum err);

#define CHECK_OPENGL_ERROR(x)                                                                                          \
  GLenum err_##x = glGetError();                                                                                       \
  if (GL_NO_ERROR != err_##x) {                                                                                        \
    printf("\n ~~ ERROR ~~ %i: %s \n", x, gl_error_to_string(err_##x));                                                \
  }

inline void
print_gpu_info()
{
  // how to get GPU info from OpenGL
  // -------------------------------
  // GPU Info
  printf("(INFO) OpenGL version supported by this platform (%s): \n", glGetString(GL_VERSION));
  int params[1];
  glGetIntegerv(GL_MAX_SHADER_STORAGE_BUFFER_BINDINGS, params);
  // printf("(INFO) GPU: Max shader storage buffer bindings: %i \n", params[0]);
  glGetIntegerv(GL_MAX_TEXTURE_IMAGE_UNITS, params);
  printf("(INFO) GPU: Max texture image units: %i \n", params[0]);

  // GL_MAX_SHADER_STORAGE_BUFFER_BINDINGS = 96
  // GL_MAX_SHADER_STORAGE_BLOCK_SIZE = 2147483647
  // GL_MAX_VERTEX_SHADER_STORAGE_BLOCKS = 16
  // GL_MAX_FRAGMENT_SHADER_STORAGE_BLOCKS = 16
  // GL_MAX_GEOMETRY_SHADER_STORAGE_BLOCKS = 16
  // GL_MAX_TESS_CONTROL_SHADER_STORAGE_BLOCKS = 16
  // GL_MAX_TESS_EVALUATION_SHADER_STORAGE_BLOCKS = 16
  // GL_MAX_COMPUTE_SHADER_STORAGE_BLOCKS = 16
  // GL_MAX_COMBINED_SHADER_STORAGE_BLOCKS = 96
}

} // namespace fightingengine
