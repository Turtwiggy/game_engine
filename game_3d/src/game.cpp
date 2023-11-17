#include "game.hpp"

#include "audio/components.hpp"
#include "audio/system.hpp"
#include "entt/helpers.hpp"
#include "events/components.hpp"
#include "events/helpers/keyboard.hpp"
#include "events/system.hpp"
#include "imgui/helpers.hpp"
#include "maths/maths.hpp"
#include "modules/animator/components.hpp"
#include "modules/animator/helpers.hpp"
#include "modules/camera/perspective.hpp"
#include "modules/camera/system.hpp"
#include "modules/models/components.hpp"
#include "modules/models/helpers.hpp"
#include "modules/renderer/components.hpp"
#include "modules/renderer/helpers.hpp"
#include "modules/renderer/system.hpp"

#include <glm/glm.hpp>
#include <imgui.h>
#include <optick.h>

#include <string>

namespace game2d {
using namespace std::literals;

void
init(engine::SINGLETON_Application& app, entt::registry& r)
{
  // r.emplace<SINGLETON_AudioComponent>(r.create());
  r.emplace<SINGLETON_InputComponent>(r.create());
  // init_audio_system(r);
  init_input_system(r);

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

  r.emplace<SINGLE_ModelsComponent>(r.create());
  // r.emplace<SINGLE_AnimationsComponent>(r.create());
  r.emplace<SINGLE_AnimatorComponent>(r.create());

  auto& models = get_first_component<SINGLE_ModelsComponent>(r);
  auto& animator = get_first_component<SINGLE_AnimatorComponent>(r);
  load_models(models);
  load_animations(animator, models);
  play_animation(animator, &animator.animation_0_data);

  init_renderer_system(r);

  // create a car
  auto e = r.create();

  TransformComponent tc;
  tc.position = { 0.0f, 0.0f, 0.0f };
  tc.rotation = { -engine::HALF_PI, 0.0f, 0.0f };
  tc.scale = { 1.0f, 1.0f, 1.0f };
  r.emplace<TransformComponent>(e, tc);
  r.emplace<CarComponent>(e);
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

  // camera to follow car
  // const auto camera_ent = get_first<PerspectiveCamera>(r);
  // const auto& camera = r.get<PerspectiveCamera>(camera_ent);
  // if (follow_car) {
  //   auto& camera_transform = r.get<TransformComponent>(camera_ent);
  //   const glm::vec3 camera_offset = { -8, 4, 5 };
  //   camera_transform.position = position + camera_offset;
  // }

  // game logic
  const auto& input = get_first_component<SINGLETON_InputComponent>(r);
  const auto& view = r.view<TransformComponent, CarComponent>();
  for (const auto& [entity, t, car] : view.each()) {
    const float car_turn_speed = 5.0f;
    const float car_speed = 10.0f;
    const float move_velocity = car_speed * dt;
    const float turn_velocity = car_turn_speed * dt;

    // update rotation
    if (get_key_held(input, SDL_SCANCODE_LEFT))
      t.rotation.z += turn_velocity;
    if (get_key_held(input, SDL_SCANCODE_RIGHT))
      t.rotation.z -= turn_velocity;

    // get direction from rotation
    const auto fwd_dir = glm::rotate(vec3_to_quat(t.rotation), glm::vec3(1.0f, 0.0f, 0.0f));
    if (get_key_held(input, SDL_SCANCODE_UP))
      t.position += fwd_dir * move_velocity;
    if (get_key_held(input, SDL_SCANCODE_DOWN))
      t.position -= fwd_dir * move_velocity;
  }

  // rendering
  {
    auto& animator = get_first_component<SINGLE_AnimatorComponent>(r);
    update_animation(animator, dt);

    update_renderer_system(app, r);
  }

  // ui
  {
    const auto& models = r.view<CarComponent, TransformComponent>();
    ImGui::Begin("Models");
    for (const auto& [e, car, transform] : models.each()) {
      game2d::imgui_draw_vec3("Pos: ", transform.position.x, transform.position.y, transform.position.z);
      game2d::imgui_draw_vec3("Render Size: ", transform.scale.x, transform.scale.y, transform.scale.z);
      game2d::imgui_draw_vec3("Render Angle:", transform.rotation.x, transform.rotation.y, transform.rotation.z);
    }
    ImGui::End();

    const auto& camera = get_first<PerspectiveCamera>(r);
    const auto& camera_p = r.get<PerspectiveCamera>(camera);
    const auto& camera_t = r.get<TransformComponent>(camera);

    ImGui::Begin("Update");

    ImGui::Text("Camera");
    ImGui::Text("Pos %f %f %f", camera_t.position.x, camera_t.position.y, camera_t.position.z);
    ImGui::Text("Pitch/Yaw %f %f", camera_p.pitch, camera_p.yaw);
    ImGui::Separator();

    ImGui::End();

    if (ImGui::BeginMainMenuBar()) {
      ImGui::Text("%0.2f FPS", ImGui::GetIO().Framerate);

      if (ImGui::MenuItem("Quit", "Esc"))
        app.running = false;

      ImGui::EndMainMenuBar();
    }
  }
}

} // namespace game2d