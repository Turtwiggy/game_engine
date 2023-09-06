#include "game.hpp"

#include "audio/components.hpp"
#include "audio/system.hpp"
#include "entt/helpers.hpp"
#include "events/components.hpp"
#include "events/system.hpp"
#include "modules/camera/perspective.hpp"
#include "modules/camera/system.hpp"
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
init(engine::SINGLETON_Application& app, entt::registry& r)
{
  // r.emplace<SINGLETON_AudioComponent>(r.create());
  r.emplace<SINGLETON_InputComponent>(r.create());
  // init_audio_system(r);
  init_input_system(r);
  r.emplace<SINGLE_ShadersComponent>(r.create());

  auto camera_entity = r.create();
  PerspectiveCamera c;
  c.projection = calculate_perspective_projection(app.width, app.height);
  r.emplace<PerspectiveCamera>(camera_entity, c);
  r.emplace<TransformComponent>(camera_entity);

  glEnable(GL_DEPTH_TEST);

  // glEnable(GL_BLEND);
  // glEnable(GL_DEPTH_TEST);
  // glEnable(GL_MULTISAMPLE);
  // glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
  // SDL_GL_SetAttribute(SDL_GL_MULTISAMPLEBUFFERS, 1);
  // SDL_GL_SetAttribute(SDL_GL_MULTISAMPLESAMPLES, 4);
  // SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
  // SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);
  // SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, 8);

  SINGLE_RendererComponent renderer;

  // set up vertex data (and buffer(s)) and configure vertex attributes
  // ------------------------------------------------------------------
  const float vertices[] = {
    // clang-format off
        -0.5f, -0.5f, -0.5f,  0.0f, 0.0f,
         0.5f, -0.5f, -0.5f,  1.0f, 0.0f,
         0.5f,  0.5f, -0.5f,  1.0f, 1.0f,
         0.5f,  0.5f, -0.5f,  1.0f, 1.0f,
        -0.5f,  0.5f, -0.5f,  0.0f, 1.0f,
        -0.5f, -0.5f, -0.5f,  0.0f, 0.0f,

        -0.5f, -0.5f,  0.5f,  0.0f, 0.0f,
         0.5f, -0.5f,  0.5f,  1.0f, 0.0f,
         0.5f,  0.5f,  0.5f,  1.0f, 1.0f,
         0.5f,  0.5f,  0.5f,  1.0f, 1.0f,
        -0.5f,  0.5f,  0.5f,  0.0f, 1.0f,
        -0.5f, -0.5f,  0.5f,  0.0f, 0.0f,

        -0.5f,  0.5f,  0.5f,  1.0f, 0.0f,
        -0.5f,  0.5f, -0.5f,  1.0f, 1.0f,
        -0.5f, -0.5f, -0.5f,  0.0f, 1.0f,
        -0.5f, -0.5f, -0.5f,  0.0f, 1.0f,
        -0.5f, -0.5f,  0.5f,  0.0f, 0.0f,
        -0.5f,  0.5f,  0.5f,  1.0f, 0.0f,

         0.5f,  0.5f,  0.5f,  1.0f, 0.0f,
         0.5f,  0.5f, -0.5f,  1.0f, 1.0f,
         0.5f, -0.5f, -0.5f,  0.0f, 1.0f,
         0.5f, -0.5f, -0.5f,  0.0f, 1.0f,
         0.5f, -0.5f,  0.5f,  0.0f, 0.0f,
         0.5f,  0.5f,  0.5f,  1.0f, 0.0f,

        -0.5f, -0.5f, -0.5f,  0.0f, 1.0f,
         0.5f, -0.5f, -0.5f,  1.0f, 1.0f,
         0.5f, -0.5f,  0.5f,  1.0f, 0.0f,
         0.5f, -0.5f,  0.5f,  1.0f, 0.0f,
        -0.5f, -0.5f,  0.5f,  0.0f, 0.0f,
        -0.5f, -0.5f, -0.5f,  0.0f, 1.0f,

        -0.5f,  0.5f, -0.5f,  0.0f, 1.0f,
         0.5f,  0.5f, -0.5f,  1.0f, 1.0f,
         0.5f,  0.5f,  0.5f,  1.0f, 0.0f,
         0.5f,  0.5f,  0.5f,  1.0f, 0.0f,
        -0.5f,  0.5f,  0.5f,  0.0f, 0.0f,
        -0.5f,  0.5f, -0.5f,  0.0f, 1.0f
    // clang-format on
  };

  renderer.cubes.push_back({ 0.0f, 0.0f, 0.0f });
  renderer.cubes.push_back({ 2.0f, 5.0f, -15.0f });
  renderer.cubes.push_back({ -1.5f, -2.2f, -2.5f });
  renderer.cubes.push_back({ -3.8f, -2.0f, -12.3f });
  renderer.cubes.push_back({ 2.4f, -0.4f, -3.5f });
  renderer.cubes.push_back({ -1.7f, 3.0f, -7.5f });
  renderer.cubes.push_back({ 1.3f, -2.0f, -2.5f });
  renderer.cubes.push_back({ 1.5f, 2.0f, -2.5f });
  renderer.cubes.push_back({ 1.5f, 0.2f, -1.5f });
  renderer.cubes.push_back({ -1.3f, 1.0f, -1.5f });

  auto& VAO = renderer.cube_vao;
  auto& VBO = renderer.cube_vbo;
  glGenVertexArrays(1, &VAO);
  glGenBuffers(1, &VBO);
  glBindVertexArray(VAO);
  glBindBuffer(GL_ARRAY_BUFFER, VBO);
  glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
  // position
  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
  glEnableVertexAttribArray(0);
  // texture coord
  glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
  glEnableVertexAttribArray(1);

  r.emplace<SINGLE_RendererComponent>(r.create(), renderer);
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
    update_camera_system(app.window, r, dt);
  }

  // rendering
  const engine::LinearColour background_col(0.8f, 0.8f, 0.8f, 1.0f);
  {
    Framebuffer::default_fbo();
    RenderCommand::set_viewport(0, 0, app.width, app.height);
    RenderCommand::set_clear_colour_linear(background_col);
    RenderCommand::clear();

    const auto& camera = get_first_component<PerspectiveCamera>(r);
    const auto& renderer = get_first_component<SINGLE_RendererComponent>(r);
    auto& shaders = get_first_component<SINGLE_ShadersComponent>(r);
    shaders.basic.bind();
    shaders.basic.set_mat4("projection", camera.projection);
    shaders.basic.set_mat4("view", camera.view);

    // draw sum cubes
    glBindVertexArray(renderer.cube_vao);
    for (int i = 0; auto& cube : renderer.cubes) {
      i++;
      glm::mat4 model(1.0f);
      model = glm::translate(model, cube);
      float angle = 20.0f * i;
      model = glm::rotate(model, glm::radians(angle), glm::vec3(1.0f, 0.3f, 0.5f));
      shaders.basic.set_mat4("model", model);
      glDrawArrays(GL_TRIANGLES, 0, 36);
    }

    //
  }

  // ui
  {
    const auto& camera = get_first<PerspectiveCamera>(r);
    const auto& camera_p = r.get<PerspectiveCamera>(camera);
    const auto& camera_t = r.get<TransformComponent>(camera);

    ImGui::Begin("Update");

    ImGui::Text("Camera %f %f %f", camera_t.position.x, camera_t.position.y, camera_t.position.z);
    ImGui::Text("Camera Pitch/Yaw %f %f", camera_p.pitch, camera_p.yaw);

    if (ImGui::Button("Quit"))
      app.running = false;
    ImGui::End();
  }
}

} // namespace game2d