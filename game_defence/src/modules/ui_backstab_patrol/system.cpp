#include "system.hpp"

#include "entt/helpers.hpp"
#include "events/helpers/mouse.hpp"
#include "lifecycle/components.hpp"
#include "modules/actor_enemy/components.hpp"
#include "modules/actor_player/components.hpp"
#include "modules/actor_turret/helpers.hpp"
#include "modules/resolve_collisions/helpers.hpp"
#include "modules/scene/components.hpp"
#include "modules/scene/helpers.hpp"
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

  // HACK: if you collide with anything, lose
  //
  const auto& physics = get_first_component<SINGLETON_PhysicsComponent>(r);
  for (const auto& coll : physics.collision_stay) {
    const auto a = static_cast<entt::entity>(coll.ent_id_0);
    const auto b = static_cast<entt::entity>(coll.ent_id_1);
    const auto [a_player, b_group] = collision_of_interest<PlayerComponent, EnemyComponent>(r, a, b);
    if (a_player != entt::null && b_group != entt::null)
      move_to_scene_start(r, Scene::menu);
  }
};

} // namespace game2d