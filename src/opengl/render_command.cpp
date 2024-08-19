
// header
#include "opengl/render_command.hpp"
#include "colour/colour.hpp"

// other library headers
#include "deps/opengl.hpp"
#include <SDL2/SDL.h>

namespace engine {

void
RenderCommand::set_viewport(int x, int y, int width, int height)
{
  glViewport(x, y, width, height);
}

void
RenderCommand::set_clear_colour_srgb(const SRGBColour& colour)
{
  glClearColor(colour.r / 255.0f, colour.g / 255.0f, colour.b / 255.0f, colour.a);
}

void
RenderCommand::set_clear_colour_linear(const LinearColour& colour)
{
  glClearColor(colour.r, colour.g, colour.b, colour.a);
}

void
RenderCommand::clear()
{
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

} // namespace engine
