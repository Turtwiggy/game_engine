#include "ui_combat_designer_system.hpp"
#include "actors/actor_helpers.hpp"
#include "engine/entt/helpers.hpp"
#include "engine/imgui/helpers.hpp"
#include "engine/maths/grid.hpp"
#include "engine/physics/components.hpp"
#include "modules/actor_player/components.hpp"
#include "modules/camera/components.hpp"
#include "modules/combat/components.hpp"
#include "modules/raws/raws_components.hpp"
#include "modules/renderer/components.hpp"
#include "modules/ui_inventory/ui_inventory_components.hpp"

#include <imgui.h>

namespace game2d {

void
update_ui_combat_designer_system(entt::registry& r)
{
  const auto& raws = get_first_component<Raws>(r);

  ImGui::Begin("UI combat designer system");
  ImGui::Text("Hello, World");

  static float tilesize = 50.0f;
  static int gpx = 0, gpy = 0;
  imgui_draw_int("x", gpx);
  imgui_draw_int("y", gpy);

  static bool is_enemy = false;
  imgui_draw_bool("enemy", is_enemy);

  if (ImGui::Button("Spawn")) {
    const auto wp = engine::grid::grid_space_to_world_space_center({ gpx, gpy }, tilesize);

    entt::entity e = entt::null;

    if (!is_enemy) {
      e = spawn_mob(r, "dungeon_actor_hero", { 0, 0 });
      // r.emplace<CameraFollow>(e);
      r.emplace<CameraLerpToTarget>(e);
      r.emplace<CircleComponent>(e);
      r.emplace<PlayerComponent>(e);
      r.emplace<TeamComponent>(e, AvailableTeams::player);
      r.get<PhysicsBodyComponent>(e).base_speed = 100.0f;
      // r.emplace<MovementJetpackComponent>(e);
      r.emplace<InitBodyAndInventory>(e);
      spawn_particle_emitter(r, "anything", { 0, 0 }, e);
    }

    if (is_enemy) {
      e = spawn_mob(r, "dungeon_actor_enemy_default", wp);
      r.emplace<TeamComponent>(e, TeamComponent{ AvailableTeams::enemy });
    }

    set_position(r, e, wp);

    //
  }

  ImGui::Separator();
  for (const auto& key : get_raws_keys(raws))
    ImGui::Text("%s", key.c_str());

  ImGui::End();
}

} // namespace game2d