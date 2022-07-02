
// header
#include "engine/opengl/render_command.hpp"
#include "engine/colour.hpp"

// other library headers
#include <GL/glew.h>
#include <SDL2/SDL.h>

namespace engine {

void
RenderCommand::set_viewport(int x, int y, int width, int height)
{
  // printf("(render_command) setting viewport... \n");
  glViewport(x, y, width, height);
}

void
RenderCommand::set_clear_colour_srgb(const SRGBColour& colour)
{
  glClearColor(colour.r, colour.g, colour.b, colour.a);
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
