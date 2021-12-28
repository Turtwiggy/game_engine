
// header
#include "engine/opengl/util.hpp"

// c  headers
#include <cassert>
#include <stdio.h>

namespace engine {

unsigned int
next_power_of_two(unsigned int x)
{
  x--;
  x |= x >> 1;  // handle  2 bit numbers
  x |= x >> 2;  // handle  4 bit numbers
  x |= x >> 4;  // handle  8 bit numbers
  x |= x >> 8;  // handle 16 bit numbers
  x |= x >> 16; // handle 32 bit numbers
  x++;
  return x;
}

const char*
gl_error_to_string(GLenum err)
{
  switch (err) {
    case GL_NO_ERROR:
      return "GL_NO_ERROR";
    case GL_INVALID_ENUM:
      return "GL_INVALID_ENUM";
    case GL_INVALID_VALUE:
      return "GL_INVALID_VALUE";
    case GL_INVALID_OPERATION:
      return "GL_INVALID_OPERATION";
    case GL_STACK_OVERFLOW:
      return "GL_STACK_OVERFLOW";
    case GL_STACK_UNDERFLOW:
      return "GL_STACK_UNDERFLOW";
    case GL_OUT_OF_MEMORY:
      return "GL_OUT_OF_MEMORY";
    case 0x8031: /* not core */
      return "GL_TABLE_TOO_LARGE_EXT";
    case 0x8065: /* not core */
      return "GL_TEXTURE_TOO_LARGE_EXT";
    case GL_INVALID_FRAMEBUFFER_OPERATION:
      return "GL_INVALID_FRAMEBUFFER_OPERATION";
    default:
      assert(!"Unhandled GL error code");
      return NULL;
  }
}

void
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

} // namespace engine
