#include "pch.hpp"

#include "helpers.hpp"

#include "engine/actors/actor_helpers.hpp"
#include "engine/colour/colour.hpp"
#include "engine/lifecycle/components.hpp"
#include "engine/physics/physics_helpers.hpp"
#include "modules/combat/components.hpp"
#include "modules/core_animations/rotate_components.hpp"
#include "modules/core_raws/raws_components.hpp"
#include "modules/event_coll_player_xp/event_coll_player_xp_components.hpp"
#include "modules/ui_colours/ui_colours_helpers.hpp"

namespace game2d {

auto pink = hex_to_srgb("#D075CE"); // pink

void
drop_levelup_xp_on_death_callback(entt::registry& r, const entt::entity e)
{
  const auto size = glm::vec2{ 16, 16 };

  const auto item_e = spawn(r, "item_xp");
  give_life(r, item_e, get_position(r, e), size);
  r.emplace<TeamComponent>(item_e, AvailableTeams::neutral);
  r.emplace<AnimationRotate>(item_e);
  set_colour(r, item_e, pink);

  auto fixture_e = get_fixture_by_tag(r, item_e, "fixture_item");
  r.emplace<XpComponent>(fixture_e, XpComponent{ .levelup = true });

  // xp doesnt do anything on it's death
  r.remove<OnDeathCallbacks>(item_e);
};

void
drop_xp_on_death_callback(entt::registry& r, const entt::entity e)
{
  const auto size = glm::vec2{ 8, 8 };

  const auto item_e = spawn(r, "item_xp");
  give_life(r, item_e, get_position(r, e), size);
  r.emplace<TeamComponent>(item_e, AvailableTeams::neutral);

  auto fixture_e = get_fixture_by_tag(r, item_e, "fixture_item");
  r.emplace<XpComponent>(fixture_e);

  // xp doesnt do anything on it's death
  r.remove<OnDeathCallbacks>(item_e);
};

} // namespace game2d