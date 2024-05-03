#include "system.hpp"

#include "entt/helpers.hpp"
#include "events/helpers/mouse.hpp"
#include "lifecycle/components.hpp"
#include "modules/actor_enemy/components.hpp"
#include "modules/actor_player/components.hpp"
#include "modules/actor_turret/helpers.hpp"
#include "modules/resolve_collisions/helpers.hpp"
#include "modules/ui_worldspace_text/components.hpp"

#include "imgui.h"
#include "physics/components.hpp"

namespace game2d {

// show worldspace ui for PatrolComponent when in range
void
update_ui_backstab_patrol_system(entt::registry& r)
{
  const auto& player_e = get_first<PlayerComponent>(r);
  if (player_e == entt::null)
    return;
  // const auto& player_aabb = r.get<AABB>(player_e);
  auto& dead = get_first_component<SINGLETON_EntityBinComponent>(r);

  // get enemies within stabbin" range.
  //
  const auto enemies = get_within_range<EnemyComponent>(r, player_e, 1000 * 1000);

  ImGui::Begin("Debug__BackstabPatrolSystem");
  ImGui::Text("Backstabbable Enemies in range: %i", enemies.size());

  const bool click = get_mouse_lmb_press();

  // Assume everything out of range

  // BUG: wouuld remove all worldspace-ui including things not being backstabbed
  const auto& worldspace_ui_view = r.view<WorldspaceTextComponent>();
  for (const auto& [e, text] : worldspace_ui_view.each())
    text.text = "-";

  for (const auto& [e, distance] : enemies) {
    const auto eid = static_cast<uint32_t>(e);
    ImGui::PushID(eid);

    auto& ui_txt = r.get_or_emplace<WorldspaceTextComponent>(e);
    ui_txt.text = std::to_string(distance);

    if (distance < 10000 && click) // kill them all!
      dead.dead.emplace(e);

    ImGui::PopID();
  }
  ImGui::End();

  // const auto& map_e = get_first<MapComponent>(r);
  // if (map_e == entt::null)
  //   return;
  // const auto& map = get_first_component<MapComponent>(r);

  //
  // HACK: stuff below here shouldnt be here
  //

  // Convert Map to Grid (?)
  // GridComponent grid;
  // grid.size = map.tilesize;
  // grid.width = map.xmax;
  // grid.height = map.ymax;
  // grid.grid = map.map;

  // const static auto convert_position_to_gridspace = [&map](const glm::ivec2 pos) -> glm::ivec2 {
  //   auto gridpos = engine::grid::world_space_to_grid_space(pos, map.tilesize);
  //   gridpos.x = glm::clamp(gridpos.x, 0, map.xmax - 1);
  //   gridpos.y = glm::clamp(gridpos.y, 0, map.ymax - 1);
  //   return gridpos;
  // };
  // const static auto convert_position_to_index = [&map](const glm::ivec2& pos) -> int {
  //   const auto gridpos = convert_position_to_gridspace(pos);
  //   return engine::grid::grid_position_to_index(gridpos, map.xmax);
  // };

  const auto& physics = get_first_component<SINGLETON_PhysicsComponent>(r);
  for (const auto& coll : physics.collision_stay) {
    const auto a = static_cast<entt::entity>(coll.ent_id_0);
    const auto b = static_cast<entt::entity>(coll.ent_id_1);
    const auto [a_player, b_group] = collision_of_interest<PlayerComponent, EnemyComponent>(r, a, b);
    if (a_player != entt::null && b_group != entt::null) {
      const auto& a_vel = r.get<VelocityComponent>(a_player); // vel of player
      const auto& b_vel = r.get<VelocityComponent>(b_group);  // vel of patrol

      // TODO: fix if you're moving away, and the enemy backstabs you counting as "approaching"
      const float dot_product = glm::dot(glm::vec2{ a_vel.x, a_vel.y }, { b_vel.x, b_vel.y });
      const bool backstabbed = dot_product <= 0.0f; // opposite directions
      std::cout << "backstabbed: " << backstabbed << std::endl;

      // replace with going to "dungeon" scene
      // move_to_scene_start(r, Scene::warhammer);
    }
  }
};

} // namespace game2d