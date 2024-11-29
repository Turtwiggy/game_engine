#include "ui_brawler_editor_system.hpp"

#include "actors/actor_helpers.hpp"
#include "engine/entt/helpers.hpp"
#include "engine/events/components.hpp"
#include "engine/events/helpers/keyboard.hpp"
#include "engine/physics/components.hpp"
#include "engine/physics/helpers.hpp"
#include "engine/sprites/components.hpp"
#include "imgui.h"
#include "modules/colour/components.hpp"
#include <SDL_scancode.h>
#include <box2d/b2_body.h>

namespace game2d {

void
update_ui_brawler_editor_system(entt::registry& r, const glm::ivec2 mouse_pos)
{
#if !defined(_DEBUG)
  return;
#endif
  const auto& input_c = get_first_component<SINGLE_InputComponent>(r);

  ImGui::Begin("UI brawler editor system");

  auto draw_wall_key = SDL_SCANCODE_E;

  static entt::entity e = entt::null;
  static glm::vec2 size{ 0, 0 };
  static glm::vec2 press_loc{ 0, 0 };
  static glm::vec2 center{ 0, 0 };

  ImGui::Text("size: %f %f", size.x, size.y);
  ImGui::Text("center: %f %f", center.x, center.y);
  ImGui::Text("press_loc: %f %f", press_loc.x, press_loc.y);

  if (get_key_down(input_c, draw_wall_key)) {
    SDL_Log("press...");
    size = { 0, 0 };
    center = { 0, 0 };
    press_loc = mouse_pos;

    e = r.create();
    r.emplace<TagComponent>(e, "wall");
    r.emplace<SpriteComponent>(e);
    r.emplace<TransformComponent>(e);
    r.emplace<DefaultColour>(e);
  }

  if (get_key_held(input_c, draw_wall_key)) {
    size = glm::abs(press_loc - glm::vec2(mouse_pos));
    center = press_loc + size * glm::vec2(0.5f, 0.5f);
    set_position(r, e, center);
    set_size(r, e, size);
  }

  if (get_key_up(input_c, draw_wall_key)) {
    SDL_Log("release...");

    // On release, add items to physics system
    PhysicsDescription pdesc;
    pdesc.type = b2_staticBody;
    pdesc.size = size;
    pdesc.position = center;
    create_physics_actor(r, e, pdesc);
  }

  ImGui::End();
}

} // namespace game2d