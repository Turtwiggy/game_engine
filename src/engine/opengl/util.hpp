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

inline bool
CHECK_OPENGL_ERROR(int code)
{
  GLenum err = glGetError();
  const bool opengl_error = GL_NO_ERROR != err;
  if (opengl_error)
    SDL_Log("%s", std::format("~~ ERROR ~~ {}: {}", err, gl_error_to_string(err)).c_str());
  return opengl_error;
};

void
print_gpu_info();

} // namespace engine
