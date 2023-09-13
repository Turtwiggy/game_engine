#include "game.hpp"
#include "cli.hpp"

#include "audio/components.hpp"
#include "audio/system.hpp"
#include "entt/helpers.hpp"
#include "events/components.hpp"
#include "events/helpers/keyboard.hpp"
#include "events/system.hpp"
#include "maths/maths.hpp"
#include "modules/camera/perspective.hpp"
#include "modules/camera/system.hpp"
#include "modules/gns_networking/system.hpp"
#include "modules/gns_ui_networking/components.hpp"
#include "modules/gns_ui_networking/system.hpp"
#include "modules/renderer/components.hpp"

// engine headers
#include "opengl/framebuffer.hpp"
#include "opengl/render_command.hpp"
#include "opengl/shader.hpp"
#include "opengl/texture.hpp"
#include "opengl/util.hpp"
using namespace engine; // also used for macro

#include <GL/glew.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <imgui.h>
#include <optick.h>

namespace game2d {

void
init(engine::SINGLETON_Application& app, entt::registry& r, const Cli& cli)
{
  r.emplace<SINGLE_ShadersComponent>(r.create());
  r.emplace<SINGLETON_AudioComponent>(r.create());
  r.emplace<SINGLETON_InputComponent>(r.create());
  init_audio_system(r);
  init_input_system(r);

  SINGLETON_NetworkingUIComponent networking_ui;
  networking_ui.start_server |= cli.server;
  networking_ui.server_port = cli.server_port;
  r.emplace<SINGLETON_NetworkingUIComponent>(r.create(), networking_ui);
  init_networking_system(r);

  auto camera_entity = r.create();
  PerspectiveCamera c;
  c.projection = calculate_perspective_projection(app.width, app.height);
  c.pitch = 0.37f;
  c.yaw = 1.0f;
  r.emplace<PerspectiveCamera>(camera_entity, c);
  r.emplace<TransformComponent>(camera_entity);

  glEnable(GL_DEPTH_TEST);
  // glBindTextureUnit(tex_unit, tex_id);
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
  update_networking_system(r, milliseconds_dt);
}

void
update(engine::SINGLETON_Application& app, entt::registry& r, const float dt)
{
  // poll for inputs
  {
    OPTICK_EVENT("(update)-game-tick");
    update_input_system(app, r);
    update_camera_system(app.window, r, dt);
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
    if (ImGui::BeginMainMenuBar()) {
      float framerate = ImGui::GetIO().Framerate;
      float framerate_ms = 1000.0f / ImGui::GetIO().Framerate;
      std::stringstream stream;
      stream << std::fixed << std::setprecision(2) << framerate;
      std::string framerate_str = stream.str();
      stream.str(std::string());
      stream << std::fixed << std::setprecision(2) << framerate;
      std::string framerate_ms_str = stream.str();
      std::string framerate_label = framerate_str + std::string(" FPS (") + framerate_ms_str + std::string(" ms)");
      ImGui::Text(framerate_label.c_str());

      if (ImGui::MenuItem("Quit", "Esc"))
        app.running = false;

      ImGui::EndMainMenuBar();
    }

    update_ui_networking_system(r);
  }
}

} // namespace game2d