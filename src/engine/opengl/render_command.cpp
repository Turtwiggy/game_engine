
// header
#include "engine/opengl/render_command.hpp"

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
RenderCommand::set_clear_colour(const glm::vec4& color)
{
  glClearColor(color.r, color.g, color.b, color.a);
}

void
RenderCommand::clear()
{
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

} // namespace engine
