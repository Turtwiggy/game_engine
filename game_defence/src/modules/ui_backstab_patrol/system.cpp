#include "system.hpp"

#include "entt/helpers.hpp"
#include "lifecycle/components.hpp"
#include "modules/actor_enemy/components.hpp"
#include "modules/actor_player/components.hpp"
#include "modules/actor_turret/helpers.hpp"
#include "modules/ui_worldspace_text/components.hpp"

#include "imgui.h"

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

    if (distance < 10000) {
      if (ImGui::Button("Backstab")) {
        dead.dead.emplace(e);
      }
    }

    ImGui::PopID();
  }

  ImGui::End();
};

} // namespace game2d