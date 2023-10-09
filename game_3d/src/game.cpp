#include "game.hpp"

#include "audio/components.hpp"
#include "audio/system.hpp"
#include "entt/helpers.hpp"
#include "events/components.hpp"
#include "events/helpers/keyboard.hpp"
#include "events/system.hpp"
#include "maths/maths.hpp"
#include "modules/animator/components.hpp"
#include "modules/camera/perspective.hpp"
#include "modules/camera/system.hpp"
#include "modules/models/components.hpp"
#include "modules/models/helpers.hpp"
#include "modules/renderer/components.hpp"
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

  SINGLE_ModelsComponent models;
  load_models(models);
  r.emplace<SINGLE_ModelsComponent>(r.create(), models);
  r.emplace<SINGLE_AnimationsComponent>(r.create());
  r.emplace<SINGLE_AnimatorComponent>(r.create());

  init_renderer_system(r);

  // create a car
  auto e = r.create();

  TransformComponent tc;
  tc.position = { 0.0f, 0.0f, 0.0f };
  tc.rotation = { 0.0f, 0.0f, 0.0f };
  tc.scale = { 0.5f, 0.5f, 0.5f };
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

    const glm::vec2 xy = engine::angle_radians_to_direction(t.rotation.x);
    const glm::vec3 dir = { xy.x, 0.0f, -xy.y };

    if (get_key_held(input, SDL_SCANCODE_UP))
      t.position -= dir * move_velocity;
    if (get_key_held(input, SDL_SCANCODE_DOWN))
      t.position += dir * move_velocity;
    if (get_key_held(input, SDL_SCANCODE_LEFT))
      t.rotation.x += turn_velocity;
    if (get_key_held(input, SDL_SCANCODE_RIGHT))
      t.rotation.x -= turn_velocity;
  }

  // rendering
  {
    update_renderer_system(app, r);
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