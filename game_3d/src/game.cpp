#include "game.hpp"

#include "audio/components.hpp"
#include "audio/system.hpp"
#include "entt/helpers.hpp"
#include "events/components.hpp"
#include "events/helpers/keyboard.hpp"
#include "events/system.hpp"
#include "maths/maths.hpp"
#include "modules/animator/components.hpp"
#include "modules/animator/helpers.hpp"
#include "modules/camera/perspective.hpp"
#include "modules/camera/system.hpp"
#include "modules/models/components.hpp"
#include "modules/models/helpers.hpp"
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
opengl_message_callback(GLenum source,
                        GLenum type,
                        GLuint id,
                        GLenum severity,
                        GLsizei length,
                        GLchar const* message,
                        void const* user_param)
{
  auto const src_str = [source]() {
    switch (source) {
      case GL_DEBUG_SOURCE_API:
        return "API";
      case GL_DEBUG_SOURCE_WINDOW_SYSTEM:
        return "WINDOW SYSTEM";
      case GL_DEBUG_SOURCE_SHADER_COMPILER:
        return "SHADER COMPILER";
      case GL_DEBUG_SOURCE_THIRD_PARTY:
        return "THIRD PARTY";
      case GL_DEBUG_SOURCE_APPLICATION:
        return "APPLICATION";
      case GL_DEBUG_SOURCE_OTHER:
        return "OTHER";
    }
  }();

  auto const type_str = [type]() {
    switch (type) {
      case GL_DEBUG_TYPE_ERROR:
        return "ERROR";
      case GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR:
        return "DEPRECATED_BEHAVIOR";
      case GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR:
        return "UNDEFINED_BEHAVIOR";
      case GL_DEBUG_TYPE_PORTABILITY:
        return "PORTABILITY";
      case GL_DEBUG_TYPE_PERFORMANCE:
        return "PERFORMANCE";
      case GL_DEBUG_TYPE_MARKER:
        return "MARKER";
      case GL_DEBUG_TYPE_OTHER:
        return "OTHER";
    }
  }();

  auto const severity_str = [severity]() {
    switch (severity) {
      case GL_DEBUG_SEVERITY_NOTIFICATION:
        return "NOTIFICATION";
      case GL_DEBUG_SEVERITY_LOW:
        return "LOW";
      case GL_DEBUG_SEVERITY_MEDIUM:
        return "MEDIUM";
      case GL_DEBUG_SEVERITY_HIGH:
        return "HIGH";
    }
  }();
  std::cout << src_str << ", " << type_str << ", " << severity_str << ", " << id << ": " << message << '\n';
};

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
  c.pitch = 0.37f;
  c.yaw = 1.0f;
  r.emplace<PerspectiveCamera>(camera_entity, c);
  TransformComponent camera_transform;
  camera_transform.position.x = -3.8;
  camera_transform.position.y = 2.4;
  camera_transform.position.z = 2.85;
  r.emplace<TransformComponent>(camera_entity, camera_transform);

  SINGLE_ModelsComponent models;
  load_models(models);
  r.emplace<SINGLE_ModelsComponent>(r.create(), models);
  r.emplace<SINGLE_AnimationsComponent>(r.create());
  r.emplace<SINGLE_AnimatorComponent>(r.create());

  // glEnable(GL_DEBUG_OUTPUT);
  // glDebugMessageCallback(opengl_message_callback, nullptr);
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

  SINGLE_RendererComponent renderer;

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

  engine::RandomState rnd;
  for (int i = 0; i < 100; i++) {
    float rnd_x = engine::rand_det_s(rnd.rng, -50, 50);
    float rnd_z = engine::rand_det_s(rnd.rng, -50, 50);
    renderer.cubes.push_back({ rnd_x, 0.5f, rnd_z });
  }
  for (int i = 0; i < 100; i++) {
    float rnd_x = engine::rand_det_s(rnd.rng, -50, 50);
    float rnd_z = engine::rand_det_s(rnd.rng, -50, 50);
    renderer.lights.push_back({ rnd_x, 0.5f, rnd_z });
  }

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

  // temp: car
  static bool follow_car = true;
  static glm::vec3 position(0.0f, 0.0f, 0.0f);
  static glm::vec3 rotation(0.0f, 0.0f, 0.0f);
  static glm::vec3 scale(0.5f, 0.5f, 0.5f);

  // rendering
  const engine::LinearColour background_col(0.8f, 0.8f, 0.8f, 1.0f);
  {
    Framebuffer::default_fbo();
    RenderCommand::set_viewport(0, 0, app.width, app.height);
    RenderCommand::set_clear_colour_linear(background_col);
    RenderCommand::clear();

    // camera to follow car

    const auto camera_ent = get_first<PerspectiveCamera>(r);
    const auto& camera = r.get<PerspectiveCamera>(camera_ent);
    if (follow_car) {
      auto& camera_transform = r.get<TransformComponent>(camera_ent);
      const glm::vec3 camera_offset = { -8, 4, 5 };
      camera_transform.position = position + camera_offset;
    }

    const auto& renderer = get_first_component<SINGLE_RendererComponent>(r);
    auto& shaders = get_first_component<SINGLE_ShadersComponent>(r);

    // RENDER CUBES
    {
      glBindVertexArray(renderer.cube_vao);
      // lights
      shaders.solid_colour.bind();
      shaders.solid_colour.set_mat4("projection", camera.projection);
      shaders.solid_colour.set_mat4("view", camera.view);
      for (const auto& c : renderer.lights) {
        glm::mat4 model(1.0f);
        model = glm::translate(model, c);
        model = glm::scale(model, { 1.0f, 1.0f, 1.0f });
        shaders.solid_colour.set_mat4("model", model);
        glDrawArrays(GL_TRIANGLES, 0, 36);
      }

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

    // draw an animated model
    {
      const auto& animator = get_first_component<SINGLE_AnimatorComponent>(r);
      const auto& models = get_first_component<SINGLE_ModelsComponent>(r);
      shaders.animated.bind();
      shaders.animated.set_mat4("projection", camera.projection);
      shaders.animated.set_mat4("view", camera.view);
      const auto& transforms = animator.final_bone_matrices;
      // for (int i = 0; i < transforms.size(); ++i)
      //   shaders.animated.set_mat4("final_bone_matrices[" + std::to_string(i) + "]", transforms[i]);

      glm::mat4 model(1.0f);

      const float car_turn_speed = 5.0f;
      const float car_speed = 10.0f;
      const float move_velocity = car_speed * dt;
      const float turn_velocity = car_turn_speed * dt;

      const glm::vec2 xy = engine::angle_radians_to_direction(rotation.x);
      const glm::vec3 dir = { xy.x, 0.0f, -xy.y };

      auto& input = get_first_component<SINGLETON_InputComponent>(r);
      if (get_key_held(input, SDL_SCANCODE_UP))
        position -= dir * move_velocity;
      if (get_key_held(input, SDL_SCANCODE_DOWN))
        position += dir * move_velocity;
      if (get_key_held(input, SDL_SCANCODE_LEFT))
        rotation.x += turn_velocity;
      if (get_key_held(input, SDL_SCANCODE_RIGHT))
        rotation.x -= turn_velocity;

      model = glm::translate(model, position);
      model = glm::scale(model, scale);
      model = glm::rotate(model, rotation.x, glm::vec3(0.0f, 1.0f, 0.0f));

      shaders.animated.set_mat4("model", model);
      draw_model(models.low_poly_car);
    }

    //
  }

  // ui
  {
    const auto& camera = get_first<PerspectiveCamera>(r);
    const auto& camera_p = r.get<PerspectiveCamera>(camera);
    const auto& camera_t = r.get<TransformComponent>(camera);

    ImGui::Begin("Update");

    ImGui::Text("Camera");
    ImGui::Text("Pos %f %f %f", camera_t.position.x, camera_t.position.y, camera_t.position.z);
    ImGui::Text("Pitch/Yaw %f %f", camera_p.pitch, camera_p.yaw);
    ImGui::Separator();

    ImGui::Text("Car");
    ImGui::Text("Pos %f %f %f", position.x, position.y, position.z);
    ImGui::Text("Rotation %f %f %f", rotation.x, rotation.y, rotation.z);

    ImGui::Checkbox("Follow car", &follow_car);

    ImGui::End();

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
  }
}

} // namespace game2d