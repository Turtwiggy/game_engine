#include "system.hpp"

#include "entt/helpers.hpp"
#include "modules/actor_enemy/components.hpp"
#include "modules/actor_enemy_patrol/components.hpp"
#include "modules/actor_player/components.hpp"
#include "modules/system_turnbased_enemy/components.hpp"
#include "modules/ui_worldspace_text/components.hpp"

// #include "imgui.h"

namespace game2d {
using namespace std::literals;

// show worldspace ui for PatrolComponent when in range
void
update_ui_backstab_patrol_system(entt::registry& r)
{
  const auto& player_e = get_first<PlayerComponent>(r);
  if (player_e == entt::null)
    return;

  // get enemies within stabbin" range.
  // const auto enemies = get_within_range<EnemyComponent>(r, player_e, 1000 * 1000);

  // ImGui::Begin("Debug__BackstabPatrolSystem");
  // ImGui::Text("Backstabbable Enemies in range: %i", enemies.size());

  // BUG: updates all worldspace-ui
  // const auto& worldspace_ui_view = r.view<WorldspaceTextComponent>();
  // for (const auto& [e, text] : worldspace_ui_view.each())
  //   text.text = "-";
  // set some worldspace ui

  const auto enemies_view = r.view<EnemyComponent, PatrolComponent>();
  for (const auto& [e, e_c, p_c] : enemies_view.each()) {
    const auto eid = static_cast<uint32_t>(e);
    // ImGui::PushID(eid);
    // ImGui::Text("AABB: %i %i", aabb_c.size.x, aabb_c.size.y);

    auto& ui_txt = r.get_or_emplace<WorldspaceTextComponent>(e);
    ui_txt.text = std::to_string(p_c.strength);
    ui_txt.offset = { 5, -12 }; // dont cover sprite

    // ImGui::PopID();
  }

  // ImGui::End();
};

} // namespace game2d