#include "show_tiles_in_range_helpers.hpp"

#include "actors/actor_helpers.hpp"
#include "engine/algorithm_astar_pathfinding/astar_helpers.hpp"
#include "engine/events/components.hpp"
#include "engine/maths/grid.hpp"
#include "modules/actor_player/components.hpp"
#include "modules/map/components.hpp"
#include "modules/raws/raws_components.hpp"
#include "modules/ui_inventory/ui_inventory_components.hpp"
#include "modules/ui_inventory/ui_inventory_helpers.hpp"
#include "show_tiles_in_range_components.hpp"

namespace game2d {

glm::ivec2
round_to_nearest_axis(const glm::vec2& v)
{
  const float abs_x = std::abs(v.x);
  const float abs_y = std::abs(v.y);

  if (abs_x > abs_y)
    return glm::ivec2(v.x > 0 ? 1 : -1, 0);

  if (abs_y > abs_x)
    return glm::ivec2(0, v.y > 0 ? 1 : -1);

  // v.x == v.y
  return glm::ivec2(0, 0);
};

// XXX
// XOX
// XXX
std::vector<glm::ivec2>
get_tiles_for_knife(entt::registry& r, const MapComponent& map_c, const glm::ivec2& pos)
{
  auto tiles = generate_accessible_areas_with_diagonals(r, map_c, pos, 1);

  // remove the player's starting pos
  std::erase(tiles, pos);

  return tiles;
};

//
// e.g. OXX...
// e.g. OXXXXX....
//
std::vector<glm::ivec2>
get_tiles_in_line(entt::registry& r,
                  const MapComponent& map_c,
                  const glm::ivec2& pos,
                  const glm::vec2 look_dir,
                  const int length)
{
  // get dir from input
  const auto worldspace_pos = engine::grid::grid_space_to_world_space_center(pos, map_c.tilesize);

  // these are your clickable tiles... where you can click to shoot the shotgun
  auto clickable_tiles = generate_accessible_areas(r, map_c, pos, 1);

  // remove the player's starting pos
  std::erase(clickable_tiles, pos);

  // Which direction is the player facing?
  const auto axis = round_to_nearest_axis(look_dir);

  // limit: only in dir
  const glm::ivec2 allowed_tile = pos + axis;
  auto it = std::find(clickable_tiles.begin(), clickable_tiles.end(), allowed_tile);
  if (it == std::end(clickable_tiles))
    return {}; // now allowed tiles

  std::vector<glm::ivec2> tiles;
  const glm::ivec2 base = *it;
  tiles.push_back(base);

  auto last = base;
  for (int i = 0; i < length; i++) {
    // dont allow shot to go through walls
    const glm::ivec2 next = last + axis;
    if (edge_between_gps(r, { last.x, last.y }, { next.x, next.y }) != entt::null)
      break;
    tiles.push_back(next);
    last = next;
  }

  return tiles;
};

//  XX
// OXX
//  XX
std::vector<glm::ivec2>
get_tiles_for_shotgun(entt::registry& r, const MapComponent& map_c, const glm::ivec2& pos, const glm::vec2 look_dir)
{
  // get dir from input
  const auto worldspace_pos = engine::grid::grid_space_to_world_space_center(pos, map_c.tilesize);

  // these are your clickable tiles... where you can click to shoot the shotgun
  auto clickable_tiles = generate_accessible_areas(r, map_c, pos, 1);

  // remove the player's starting pos
  std::erase(clickable_tiles, pos);

  // Which direction is the player facing?
  const auto axis = round_to_nearest_axis(look_dir);

  // limit: only in dir
  const glm::ivec2 allowed_tile = pos + axis;
  auto it = std::find(clickable_tiles.begin(), clickable_tiles.end(), allowed_tile);
  if (it == std::end(clickable_tiles))
    return {}; // now allowed tiles

  const auto perp = glm::ivec2{ -axis.y, axis.x };
  const auto base = *it;
  auto tiles = std::vector<glm::ivec2>{ base };

  const auto base_up = base + perp;
  if (edge_between_gps(r, { base.x, base.y }, { base_up.x, base_up.y }) == entt::null)
    tiles.push_back(base_up);

  const auto base_down = base - perp;
  if (edge_between_gps(r, { base.x, base.y }, { base_down.x, base_down.y }) == entt::null)
    tiles.push_back(base_down);

  const glm::ivec2 next = base + axis;
  if (edge_between_gps(r, { base.x, base.y }, { next.x, next.y }) == entt::null) {
    tiles.push_back(next);

    const auto next_up = next + perp;
    const auto up_no_edge_a = edge_between_gps(r, { base_up.x, base_up.y }, { next_up.x, next_up.y }) == entt::null;
    const auto up_no_edge_b = edge_between_gps(r, { next.x, next.y }, { next_up.x, next_up.y }) == entt::null;
    if (up_no_edge_a && up_no_edge_b)
      tiles.push_back(next_up);

    const auto next_down = next - perp;
    const auto no_edge_a = edge_between_gps(r, { base_down.x, base_down.y }, { next_down.x, next_down.y }) == entt::null;
    const auto no_edge_b = edge_between_gps(r, { next.x, next.y }, { next_down.x, next_down.y }) == entt::null;
    if (no_edge_a && no_edge_b)
      tiles.push_back(next_down);
  }

  return tiles;
};

void
update_tiles_component(entt::registry& r,
                       entt::entity e,
                       const MapComponent& map_c,
                       const InputComponent& input_c,
                       const RangeType& t,
                       TilesComponent& c)
{
  const auto gp = get_grid_position(r, e);

  std::vector<glm::ivec2> tiles;

  if (t == RangeType::knife)
    tiles = get_tiles_for_knife(r, map_c, gp);
  if (t == RangeType::pistol)
    tiles = get_tiles_in_line(r, map_c, gp, { input_c.rx, input_c.ry }, 1);
  if (t == RangeType::plunger)
    tiles = get_tiles_in_line(r, map_c, gp, { input_c.rx, input_c.ry }, 4);
  if (t == RangeType::shotgun)
    tiles = get_tiles_for_shotgun(r, map_c, gp, { input_c.rx, input_c.ry });

  c.tiles = tiles;
};

entt::entity
get_equipped_gun(entt::registry& r, const entt::entity e)
{
  const auto& body = r.get<DefaultBody>(e);
  const auto gun_e = get_slot_type(r, body.body, InventorySlotType::gun);
  const auto gun_c = r.get<InventorySlotComponent>(gun_e);
  return gun_c.item_e;
}

int
get_damage_for_item(entt::registry& r, const entt::entity item_e)
{
  const auto& item = r.get<Item>(item_e);

  if (item.melee.has_value())
    return item.melee.value().damage;

  if (item.ranged.has_value())
    return item.ranged.value().damage; // or could be bullet in gun...

  return 0;
}

int
get_damage_for_equipped_item(entt::registry& r, const entt::entity e)
{
  const auto item_e = get_equipped_gun(r, e);

  if (item_e == entt::null)
    return 0;

  return get_damage_for_item(r, item_e);
};

} // namespace game2d