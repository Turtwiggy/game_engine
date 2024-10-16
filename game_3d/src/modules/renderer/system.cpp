#include "system.hpp"

#include "components.hpp"
#include "entt/helpers.hpp"
#include "events/components.hpp"
#include "events/helpers/keyboard.hpp"
#include "imgui/helpers.hpp"
#include "maths/maths.hpp"
#include "modules/animator/components.hpp"
#include "modules/animator/helpers.hpp"
#include "modules/camera/perspective.hpp"
#include "modules/models/components.hpp"
#include "modules/models/helpers.hpp"
#include "modules/renderer/helpers.hpp"

// engine headers
#include "opengl/framebuffer.hpp"
#include "opengl/render_command.hpp"
#include "opengl/shader.hpp"
#include "opengl/texture.hpp"
#include "opengl/util.hpp"
using namespace engine; // also used for macro

#include "imgui.h"

#include <GL/glew.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/quaternion.hpp>

namespace game2d {

void
init_renderer_system(const engine::SINGLETON_Application& app, entt::registry& r)
{
  auto& ri = get_first_component<SINGLE_RendererComponent>(r);
  r.emplace<SINGLE_ShadersComponent>(r.create());
  const glm::ivec2 screen_wh = { app.width, app.height };
  ri.viewport_size_render_at = screen_wh;
  ri.viewport_size_current = screen_wh;
  const auto& fbo_size = ri.viewport_size_render_at;

  // glEnable(GL_DEBUG_OUTPUT);
  // glDebugMessageCallback(opengl_message_callback, nullptr);

  // SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);
  SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, 8);
  SDL_GL_SetAttribute(SDL_GL_MULTISAMPLEBUFFERS, 1);
  SDL_GL_SetAttribute(SDL_GL_MULTISAMPLESAMPLES, 4);
  SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);

  // glEnable(GL_BLEND);
  // glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
  glEnable(GL_DEPTH_TEST);
  glDepthFunc(GL_LESS);

  glEnable(GL_STENCIL_TEST);
  glStencilFunc(GL_NOTEQUAL, 1, 0xFF);
  glStencilOp(GL_KEEP, GL_KEEP, GL_REPLACE);

  glEnable(GL_CULL_FACE);
  glCullFace(GL_BACK);

  glEnable(GL_MULTISAMPLE);

  // SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "1");
  // glEnable(GL_MULTISAMPLE);
  // SDL_GL_SetAttribute(SDL_GL_MULTISAMPLEBUFFERS, 1);
  // SDL_GL_SetAttribute(SDL_GL_MULTISAMPLESAMPLES, 4);
  // SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
  // SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);

  // set up vertex data (and buffer(s)) and configure vertex attributes
  // ------------------------------------------------------------------
  const float vertices[] = {
    // clang-format off
        // positions          // normals           // texture coords
        -0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  0.0f,  0.0f,
         0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  1.0f,  0.0f,
         0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  1.0f,  1.0f,
         0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  1.0f,  1.0f,
        -0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  0.0f,  1.0f,
        -0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  0.0f,  0.0f,

        -0.5f, -0.5f,  0.5f,  0.0f,  0.0f,  1.0f,  0.0f,  0.0f,
         0.5f, -0.5f,  0.5f,  0.0f,  0.0f,  1.0f,  1.0f,  0.0f,
         0.5f,  0.5f,  0.5f,  0.0f,  0.0f,  1.0f,  1.0f,  1.0f,
         0.5f,  0.5f,  0.5f,  0.0f,  0.0f,  1.0f,  1.0f,  1.0f,
        -0.5f,  0.5f,  0.5f,  0.0f,  0.0f,  1.0f,  0.0f,  1.0f,
        -0.5f, -0.5f,  0.5f,  0.0f,  0.0f,  1.0f,  0.0f,  0.0f,

        -0.5f,  0.5f,  0.5f, -1.0f,  0.0f,  0.0f,  1.0f,  0.0f,
        -0.5f,  0.5f, -0.5f, -1.0f,  0.0f,  0.0f,  1.0f,  1.0f,
        -0.5f, -0.5f, -0.5f, -1.0f,  0.0f,  0.0f,  0.0f,  1.0f,
        -0.5f, -0.5f, -0.5f, -1.0f,  0.0f,  0.0f,  0.0f,  1.0f,
        -0.5f, -0.5f,  0.5f, -1.0f,  0.0f,  0.0f,  0.0f,  0.0f,
        -0.5f,  0.5f,  0.5f, -1.0f,  0.0f,  0.0f,  1.0f,  0.0f,

         0.5f,  0.5f,  0.5f,  1.0f,  0.0f,  0.0f,  1.0f,  0.0f,
         0.5f,  0.5f, -0.5f,  1.0f,  0.0f,  0.0f,  1.0f,  1.0f,
         0.5f, -0.5f, -0.5f,  1.0f,  0.0f,  0.0f,  0.0f,  1.0f,
         0.5f, -0.5f, -0.5f,  1.0f,  0.0f,  0.0f,  0.0f,  1.0f,
         0.5f, -0.5f,  0.5f,  1.0f,  0.0f,  0.0f,  0.0f,  0.0f,
         0.5f,  0.5f,  0.5f,  1.0f,  0.0f,  0.0f,  1.0f,  0.0f,

        -0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,  0.0f,  1.0f,
         0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,  1.0f,  1.0f,
         0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,  1.0f,  0.0f,
         0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,  1.0f,  0.0f,
        -0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,  0.0f,  0.0f,
        -0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,  0.0f,  1.0f,

        -0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f,  0.0f,  1.0f,
         0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f,  1.0f,  1.0f,
         0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,  1.0f,  0.0f,
         0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,  1.0f,  0.0f,
        -0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,  0.0f,  0.0f,
        -0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f,  0.0f,  1.0f
    // clang-format on
  };

  auto& VAO = ri.cube_vao;
  auto& VBO = ri.cube_vbo;
  glGenVertexArrays(1, &VAO);
  glGenBuffers(1, &VBO);
  glBindVertexArray(VAO);
  glBindBuffer(GL_ARRAY_BUFFER, VBO);
  glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

  // configure for shader
  glBindVertexArray(VAO);
  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
  glEnableVertexAttribArray(0);
  glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
  glEnableVertexAttribArray(1);
  glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));
  glEnableVertexAttribArray(2);
  CHECK_OPENGL_ERROR(0);

  // Load user textures
  const int base_tex_unit = ri.RENDERER_TEX_UNIT_COUNT;
  int next_tex_unit = base_tex_unit;
  for (auto& tex : ri.user_textures) {
    std::cout << "loading texture..." << tex.path << std::endl;
    tex.tex_unit = next_tex_unit;
    const auto loaded_tex = engine::load_texture_linear(tex.tex_unit.unit, tex.path);
    tex.tex_id.id = bind_linear_texture(loaded_tex);
    next_tex_unit++;
  }

  // rebind();
  const auto& wh = ri.viewport_size_render_at;
  engine::RenderCommand::set_viewport(0, 0, wh.x, wh.y);

  for (const auto& tex : ri.user_textures) {
    glActiveTexture(GL_TEXTURE0 + tex.tex_unit.unit);
    glBindTexture(GL_TEXTURE_2D, tex.tex_id.id);
  }

  const int tex_unit_bob_blue = search_for_texture_unit_by_texture_path(ri, "Bob_Blue")->unit;

  const auto& shaders = get_first_component<SINGLE_ShadersComponent>(r);
  shaders.animated.bind();
  shaders.animated.set_int("texture_diffuse", tex_unit_bob_blue); // hack

  //
}

void
update_renderer_system(engine::SINGLETON_Application& app, entt::registry& r, const float dt)
{
  auto& ri = get_first_component<SINGLE_RendererComponent>(r);
  const auto camera_e = get_first<PerspectiveCamera>(r);
  const auto& camera = r.get<PerspectiveCamera>(camera_e);
  const auto& camera_t = r.get<TransformComponent>(camera_e);
  auto& shaders = get_first_component<SINGLE_ShadersComponent>(r);
  const auto& input = get_first_component<SINGLETON_InputComponent>(r);
  const auto& models_c = get_first_component<SINGLE_ModelsComponent>(r);

  static float time = 0.0f;
  time += dt;

  const engine::LinearColour background_col(0.8f, 0.8f, 0.8f, 1.0f);

  if (check_if_viewport_resize(ri)) {
    ri.viewport_size_render_at = ri.viewport_size_current;
    // rebind();
  }

  if (get_key_down(input, SDL_SCANCODE_R)) {
    std::cout << "rebinding shader..." << std::endl;
    shaders.animated.reload();
  }

  Framebuffer::default_fbo();
  RenderCommand::set_viewport(0, 0, app.width, app.height);
  RenderCommand::set_clear_colour_linear(background_col);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT); // don't forget to clear the stencil buffer!

  // draw cubes as normal
  glStencilMask(0x00);

  static glm::vec3 light_direction{ -0.2f, -1.0f, -0.3f };
  ImGui::Begin("Debug Lighting");
  imgui_draw_vec3("light_direction light_direction", light_direction);
  ImGui::End();

  // std::vector<glm::vec3> lights;
  // lights.push_back(light_position);

  // RENDER CUBES
  {
    // glBindVertexArray(ri.cube_vao);
    // // lights
    // shaders.solid_colour.bind();
    // shaders.solid_colour.set_mat4("projection", camera.projection);
    // shaders.solid_colour.set_mat4("view", camera.view);
    // for (const auto& c : lights) {
    //   glm::mat4 model(1.0f);
    //   model = glm::translate(model, c);
    //   model = glm::scale(model, { 1.0f, 1.0f, 1.0f });
    //   shaders.solid_colour.set_mat4("model", model);
    //   glDrawArrays(GL_TRIANGLES, 0, 36);
    // }

    glBindVertexArray(ri.cube_vao);
    // lit cubes
    shaders.basic.bind();
    shaders.basic.set_mat4("projection", camera.projection);
    shaders.basic.set_mat4("view", camera.view);
    for (const auto& c : ri.cubes) {
      glm::mat4 model(1.0f);
      model = glm::translate(model, c);
      shaders.basic.set_mat4("model", model);
      shaders.basic.set_vec3("u_colour", { 1.0f, 0.5f, 0.31f });
      glDrawArrays(GL_TRIANGLES, 0, 36);
    }
  }

  // 1st render pass:
  // draw objects as normal, writing to the stencil buffer
  glStencilFunc(GL_ALWAYS, 1, 0xFF);
  glStencilMask(0xFF);

  {
    const auto& animator = get_first_component<SINGLE_AnimatorComponent>(r);
    shaders.animated.bind();
    shaders.animated.set_mat4("projection", camera.projection);
    shaders.animated.set_mat4("view", camera.view);

    const auto& view = r.view<const TransformComponent, const ModelComponent>();
    for (const auto& [entity, t, model_c] : view.each()) {

      const auto& transforms = animator.final_bone_matrices;
      for (int i = 0; i < transforms.size(); ++i)
        shaders.animated.set_mat4("final_bone_matrices[" + std::to_string(i) + "]", transforms[i]);

      glm::mat4 model(1.0f);
      model = glm::translate(model, t.position);
      model = glm::scale(model, t.scale);
      model *= glm::toMat4(vec3_to_quat(t.rotation));
      shaders.animated.set_mat4("model", model);

      shaders.animated.set_vec3("light.direction", light_direction);
      shaders.animated.set_vec3("camera_pos", camera_t.position);
      // shaders.animated.set_int("texture_diffuse", ri.tex_unit_diffuse_spaceship);

      glm::vec3 lightColor;
      lightColor.x = 1.0f;
      lightColor.y = 1.0f;
      lightColor.z = 1.0f;
      glm::vec3 diffuseColor = lightColor * glm::vec3(0.9f);   // decrease the influence
      glm::vec3 ambientColor = diffuseColor * glm::vec3(0.6f); // low influence
      shaders.animated.set_vec3("light.ambient", ambientColor);
      shaders.animated.set_vec3("light.diffuse", diffuseColor);
      shaders.animated.set_vec3("light.specular", { 1.0f, 1.0f, 1.0f });

      static glm::vec3 material_ambient{ 1.0f, 1.0f, 1.0f };
      static glm::vec3 material_diffuse{ 1.0f, 1.0f, 1.0f };
      static glm::vec3 material_specular{ 0.5f, 0.5f, 0.5f };
      static float material_shininess = 32.0f;
      shaders.animated.set_vec3("material.ambient", material_ambient);
      shaders.animated.set_vec3("material.diffuse", material_diffuse);
      shaders.animated.set_vec3("material.specular", material_specular);
      shaders.animated.set_float("material.shininess", material_shininess);

      draw_model(models_c.models_to_load[0]);
    }
  }

  // 2nd. render pass:
  // now draw slightly scaled versions of the objects, this time disabling stencil writing.
  // Because the stencil buffer is now filled with several 1s. The parts of the buffer that are 1 are not drawn, thus only
  // drawing the objects' size differences, making it look like borders.
  if (false) {
    glStencilFunc(GL_NOTEQUAL, 1, 0xFF);
    glStencilMask(0x00); // disable writing to the stencil buffer
    glDisable(GL_DEPTH_TEST);

    // redraw slightly larger model
    {
      const float scale = 1.05;

      shaders.solid_colour.bind();
      shaders.solid_colour.set_mat4("projection", camera.projection);
      shaders.solid_colour.set_mat4("view", camera.view);

      const auto& view = r.view<TransformComponent, ModelComponent>();
      for (const auto& [entity, t, model_c] : view.each()) {
        const glm::vec3 scale_big = t.scale * scale;
        glm::mat4 model(1.0f);
        model = glm::translate(model, t.position);
        model = glm::scale(model, scale_big);
        model *= glm::toMat4(vec3_to_quat(t.rotation));
        shaders.solid_colour.set_mat4("model", model);

        draw_model(models_c.models_to_load[0]);
      }
    }

    // done with stencil
    glStencilMask(0xFF);
    glStencilFunc(GL_ALWAYS, 0, 0xFF);
    glEnable(GL_DEPTH_TEST);
  }

  //
}

} // namespace game2d