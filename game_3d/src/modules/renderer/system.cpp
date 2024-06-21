#include "system.hpp"

#include "components.hpp"
#include "entt/helpers.hpp"
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

#include <GL/glew.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/quaternion.hpp>

namespace game2d {

void
init_renderer_system(entt::registry& r)
{
  auto& renderer = get_first_component<SINGLE_RendererComponent>(r);

  // glEnable(GL_DEBUG_OUTPUT);
  // glDebugMessageCallback(opengl_message_callback, nullptr);

  glEnable(GL_DEPTH_TEST);
  glDepthFunc(GL_LESS);
  glEnable(GL_STENCIL_TEST);
  glStencilFunc(GL_NOTEQUAL, 1, 0xFF);
  glStencilOp(GL_KEEP, GL_KEEP, GL_REPLACE);

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

  auto& VAO = renderer.cube_vao;
  auto& VBO = renderer.cube_vbo;
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

  r.emplace<SINGLE_ShadersComponent>(r.create());
}

void
update_renderer_system(engine::SINGLETON_Application& app, entt::registry& r)
{
  const auto& renderer = get_first_component<SINGLE_RendererComponent>(r);
  const auto& models = get_first_component<SINGLE_ModelsComponent>(r);
  const auto camera_ent = get_first<PerspectiveCamera>(r);
  const auto& camera = r.get<PerspectiveCamera>(camera_ent);
  auto& shaders = get_first_component<SINGLE_ShadersComponent>(r);

  const engine::LinearColour background_col(0.8f, 0.8f, 0.8f, 1.0f);

  Framebuffer::default_fbo();
  RenderCommand::set_viewport(0, 0, app.width, app.height);
  RenderCommand::set_clear_colour_linear(background_col);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT); // don't forget to clear the stencil buffer!

  // draw cubes as normal
  glStencilMask(0x00);

  // RENDER CUBES
  {
    // glBindVertexArray(renderer.cube_vao);
    // // lights
    // shaders.solid_colour.bind();
    // shaders.solid_colour.set_mat4("projection", camera.projection);
    // shaders.solid_colour.set_mat4("view", camera.view);
    // for (const auto& c : renderer.lights) {
    //   glm::mat4 model(1.0f);
    //   model = glm::translate(model, c);
    //   model = glm::scale(model, { 1.0f, 1.0f, 1.0f });
    //   shaders.solid_colour.set_mat4("model", model);
    //   glDrawArrays(GL_TRIANGLES, 0, 36);
    // }

    glBindVertexArray(renderer.cube_vao);
    // lit cubes
    shaders.basic.bind();
    shaders.basic.set_mat4("projection", camera.projection);
    shaders.basic.set_mat4("view", camera.view);
    for (const auto& c : renderer.cubes) {
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

      draw_model(model_c.model);
    }
  }

  // 2nd. render pass:
  // now draw slightly scaled versions of the objects, this time disabling stencil writing.
  // Because the stencil buffer is now filled with several 1s. The parts of the buffer that are 1 are not drawn, thus only
  // drawing the objects' size differences, making it look like borders.
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
      draw_model(model_c.model);
    }
  }

  // done with stencil
  glStencilMask(0xFF);
  glStencilFunc(GL_ALWAYS, 0, 0xFF);
  glEnable(GL_DEPTH_TEST);

  //
}

} // namespace game2d