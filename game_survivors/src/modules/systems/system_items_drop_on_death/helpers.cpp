#include "pch.hpp"

#include "helpers.hpp"

#include "engine/actors/actor_helpers.hpp"
#include "engine/colour/colour.hpp"
#include "engine/lifecycle/components.hpp"
#include "engine/physics/physics_helpers.hpp"
#include "engine/sprites/helpers.hpp"
#include "modules/combat/combat_core/components.hpp"
#include "modules/core/animations/rotate_components.hpp"
#include "modules/core/raws/raws_components.hpp"
#include "modules/core/renderer/helpers.hpp"
#include "modules/events/event_coll_player_xp/event_coll_player_xp_components.hpp"
#include "modules/ui/ui_colours/ui_colours_helpers.hpp"
#include "resources/data.hpp"

namespace game2d {

const auto pink = hex_to_srgb("#D075CE"); // pink

void
drop_levelup_xp_on_death_callback(entt::registry& r, const entt::entity e)
{
  float size = default_map_unit_tilesize;

  const auto item_e = spawn(r, "item_xp");
  give_life(r, item_e, get_position(r, e), { size, size });
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
  float size = default_map_unit_tilesize;
  auto pos = get_position(r, e);

  const auto skull_e = spawn(r, "empty");
  give_life(r, skull_e, pos, { default_map_unit_tilesize, default_map_unit_tilesize });
  set_sprite(r, skull_e, "SKULL_AND_BONES");
  set_colour(r, skull_e, my_grey);
  set_z_index(r, skull_e, ZLayer::XP_BACK);

  const auto item_e = spawn(r, "item_xp");
  give_life(r, item_e, pos, { default_map_unit_tilesize, default_map_unit_tilesize });
  r.emplace<TeamComponent>(item_e, AvailableTeams::neutral);
  set_z_index(r, item_e, ZLayer::XP_FRONT);

  auto fixture_e = get_fixture_by_tag(r, item_e, "fixture_item");
  r.emplace<XpComponent>(fixture_e);

  // xp doesnt do anything on it's death
  r.remove<OnDeathCallbacks>(item_e);
};

} // namespace game2d