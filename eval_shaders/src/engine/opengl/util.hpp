#pragma once

#include "engine/deps/opengl.hpp"

#include <SDL2/SDL_log.h>
#include <format>

namespace engine {

// http://bits.stephan-brumme.com/roundUpToNextPowerOfTwo.html
unsigned int
next_power_of_two(unsigned int x);

const char*
gl_error_to_string(GLenum err);

#define CHECK_OPENGL_ERROR(x)                                                                                               \
  GLenum err_##x = glGetError();                                                                                            \
  const bool opengl_error##x = GL_NO_ERROR != err_##x;                                                                      \
  if (opengl_error##x)                                                                                                      \
    SDL_Log("%s", std::format("~~ ERROR ~~ {}: {}", x, gl_error_to_string(err_##x)).c_str());

void
print_gpu_info();

} // namespace engine
