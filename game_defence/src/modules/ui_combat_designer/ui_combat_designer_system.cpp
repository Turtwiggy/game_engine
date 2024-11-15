#include "ui_combat_designer_system.hpp"

#include "actors/actor_helpers.hpp"
#include "engine/entt/helpers.hpp"
#include "engine/events/components.hpp"
#include "engine/events/helpers/keyboard.hpp"
#include "engine/maths/grid.hpp"
#include "engine/physics/components.hpp"
#include "modules/actor_player/components.hpp"
#include "modules/camera/components.hpp"
#include "modules/combat/components.hpp"
#include "modules/raws/raws_components.hpp"
#include "modules/renderer/components.hpp"
#include "modules/ui_inventory/ui_inventory_components.hpp"
#include "ui_combat_designer_helpers.hpp"

#include <SDL_keyboard.h>
#include <SDL_scancode.h>
#include <imgui.h>

namespace game2d {

void
update_ui_combat_designer_system(entt::registry& r, glm::ivec2 mouse_pos)
{
  const auto& raws = get_first_component<Raws>(r);
  const auto& input = get_first_component<SINGLE_InputComponent>(r);
  const auto spawn_enemy_button = SDL_SCANCODE_E;
  const auto spawn_player_button = SDL_SCANCODE_R;
  const auto lshift = SDL_SCANCODE_LSHIFT;
  const auto pos = engine::grid::worldspace_to_clamped_world_space_center(mouse_pos, 50);

  ImGui::Begin("UI combat designer system");
  ImGui::Text("spawn enemy: lshift+%s", SDL_GetScancodeName(spawn_enemy_button));
  ImGui::Text("spawn player: lshift+%s", SDL_GetScancodeName(spawn_player_button));

  if (get_key_held(input, lshift) && get_key_down(input, spawn_enemy_button)) {

    const auto view = r.view<CameraFreeMove>();
    r.destroy(view.begin(), view.end());

    auto e = spawn_mob(r, "dungeon_actor_enemy_default", pos);
    r.emplace<TeamComponent>(e, TeamComponent{ AvailableTeams::enemy });
  }

  if (get_key_held(input, lshift) && get_key_down(input, spawn_player_button)) {

    const auto view = r.view<CameraFreeMove>();
    r.destroy(view.begin(), view.end());

    auto e = spawn_mob(r, "dungeon_actor_hero", pos);
    r.emplace<CameraLerpToTarget>(e);
    // r.emplace<CircleComponent>(e);
    r.emplace<PlayerComponent>(e);
    r.emplace<TeamComponent>(e, AvailableTeams::player);
    r.get<PhysicsBodyComponent>(e).base_speed = 100.0f;
    r.emplace<InitBodyAndInventory>(e);
    // spawn_particle_emitter(r, "anything", mouse_pos, e);

    activate_unit(r, e);
  }

  ImGui::End();
}

} // namespace game2d