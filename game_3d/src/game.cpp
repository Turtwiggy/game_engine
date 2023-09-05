#include "game.hpp"

#include "audio/components.hpp"
#include "audio/system.hpp"
#include "events/components.hpp"
#include "events/system.hpp"
using namespace game2d;

// engine headers
#include "opengl/framebuffer.hpp"
#include "opengl/render_command.hpp"
#include "opengl/shader.hpp"
#include "opengl/texture.hpp"
#include "opengl/util.hpp"
using namespace engine; // also used for macro

#include "imgui.h"
#include "optick.h"

namespace game3d {

void
init(engine::SINGLETON_Application& app, entt::registry& r)
{
  r.emplace<SINGLETON_AudioComponent>(r.create());
  r.emplace<SINGLETON_InputComponent>(r.create());
  init_audio_system(r);
  init_input_system(r);

  // glEnable(GL_BLEND);
  // glEnable(GL_DEPTH_TEST);
  // glEnable(GL_MULTISAMPLE);
  // glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
  // SDL_GL_SetAttribute(SDL_GL_MULTISAMPLEBUFFERS, 1);
  // SDL_GL_SetAttribute(SDL_GL_MULTISAMPLESAMPLES, 4);
  // SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
  // SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);
  // SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, 8);
}

void
fixed_update(entt::registry& r, const uint64_t milliseconds_dt)
{
  //
}

void
update(engine::SINGLETON_Application& app, entt::registry& r, const float dt)
{
  // poll for inputs
  {
    OPTICK_EVENT("(update)-game-tick");
    update_input_system(app, r);
  }

  // rendering
  {
    const engine::LinearColour background_col(0.8f, 0.8f, 0.8f, 1.0f);
    Framebuffer::default_fbo();
    RenderCommand::set_viewport(0, 0, app.width, app.height);
    RenderCommand::set_clear_colour_linear(background_col);
    RenderCommand::clear();
  }

  // ui
  {
    ImGui::Begin("Update");
    ImGui::Button("A button");
    ImGui::End();
  }
}

} // namespace game3d