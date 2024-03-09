#include "system.hpp"

#include "actors.hpp"
#include "components.hpp"
#include "entt/helpers.hpp"
#include "modules/actor_player/components.hpp"
#include "modules/lifecycle/components.hpp"
#include "modules/ui_worldspace_text/components.hpp"
#include "physics/components.hpp"

#include <algorithm>

namespace game2d {

void
update_ui_player_name_above_player_system(entt::registry& r)
{
  std::vector<entt::entity> no_ui;

  // get a list of all players
  const auto& view = r.view<PlayerComponent, AABB>();
  for (const auto& [e, player, player_aabb] : view.each()) {
    no_ui.push_back(e);
  }

  // get a list of all the ui
  const auto& ui_view = r.view<TransformComponent, WorldspaceTextComponent, HasParentComponent>();
  for (const auto& [e, t, wst, parent] : ui_view.each()) {
    const entt::entity parent_e = parent.parent;
    const auto parent_t = r.get<TransformComponent>(parent_e);
    const auto parent_position = parent_t.position;

    // Update the transform of the ui element
    // Imgui converts this position to its own position with SetNextWindowPos
    t.position = parent_position;
    t.position.y -= parent_t.scale.y; // shove above the sprite

    const auto hmm =
      std::remove_if(no_ui.begin(), no_ui.end(), [&parent_e](const entt::entity& player) { return parent_e == player; });
    no_ui.erase(hmm, no_ui.end());
  }

  // create the player a ui element
  for (const auto& player : no_ui) {
    const auto e = create_gameplay(r, EntityType::empty);
    r.emplace<WorldspaceTextComponent>(e, "steve");
    r.emplace<HasParentComponent>(e, player);
    auto& t = r.get<TransformComponent>(e);
    t.scale.x = 0;
    t.scale.y = 0;
  }
};

} // namespace game2d