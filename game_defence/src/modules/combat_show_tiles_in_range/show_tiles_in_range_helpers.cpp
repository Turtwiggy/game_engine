#include "show_tiles_in_range_helpers.hpp"

#include "engine/algorithm_astar_pathfinding/astar_helpers.hpp"
#include "engine/map/components.hpp"
#include "engine/maths/maths.hpp"
#include "modules/core/raws/raws_components.hpp"
#include "modules/ui_inventory/ui_inventory_components.hpp"
#include "modules/ui_inventory/ui_inventory_helpers.hpp"

namespace game2d {

// XXX
// XOX
// XXX
std::vector<glm::ivec2>
get_tiles_in_area(entt::registry& r, const MapComponent& map_c, const glm::ivec2& pos, const int range)
{
  auto tiles = generate_accessible_areas_with_diagonals(r, map_c, pos, range);

  // std::vector<glm::ivec2> tiles;
  // const auto neighbour_gp = engine::grid::get_neighbour_gridpos_with_diagonals({ pos.x, pos.y }, map_c.xmax, map_c.ymax);
  // for (const auto [dir, gp] : neighbour_gp)
  //   tiles.push_back(gp);

  // remove the player's starting pos
  std::erase(tiles, pos);

  return tiles;
};

//
// e.g. r=2.. OXX...
// e.g. r=5.. OXXXXX....
//
std::vector<glm::ivec2>
get_tiles_in_line(entt::registry& r,
                  const MapComponent& map_c,
                  const glm::ivec2& pos,
                  const glm::vec2 look_dir,
                  const int length)
{
  // clickable tiles around the position
  auto clickable_tiles = generate_accessible_areas(r, map_c, pos, 1);

  // remove the player's starting pos
  std::erase(clickable_tiles, pos);

  // Which direction is the player facing?
  const auto axis = engine::round_to_nearest_axis(look_dir);

  // limit: only in dir
  const glm::ivec2 allowed_tile = pos + axis;
  auto it = std::find(clickable_tiles.begin(), clickable_tiles.end(), allowed_tile);
  if (it == std::end(clickable_tiles))
    return {}; // no allowed tiles

  std::vector<glm::ivec2> tiles;
  const glm::ivec2 base = *it;
  tiles.push_back(base);

  auto last = base;
  for (int i = 0; i < length - 1; i++) {
    // dont allow shot to go through walls
    const glm::ivec2 next = last + axis;
    if (edge_between_gps(r, { last.x, last.y }, { next.x, next.y }) != entt::null)
      break;
    tiles.push_back(next);
    last = next;
  }

  return tiles;
};

//  12345
//  XXXXX
// OXXXXX
//  XXXXX
std::vector<glm::ivec2>
get_tiles_for_shotgun(entt::registry& r,
                      const MapComponent& map_c,
                      const glm::ivec2& pos,
                      const glm::vec2 look_dir,
                      int depth)
{
  // these are your clickable tiles... where you can click to shoot the shotgun
  auto clickable_tiles = generate_accessible_areas(r, map_c, pos, depth);

  // remove the player's starting pos
  std::erase(clickable_tiles, pos);

  // Which direction is the player facing?
  const auto axis = engine::round_to_nearest_axis(look_dir);

  // limit: only in dir
  const glm::ivec2 allowed_tile = pos + axis;
  auto it = std::find(clickable_tiles.begin(), clickable_tiles.end(), allowed_tile);
  if (it == std::end(clickable_tiles))
    return {}; // now allowed tiles

  const auto perp = glm::ivec2{ -axis.y, axis.x };
  auto tiles = std::vector<glm::ivec2>{};

  const auto base = *it;
  tiles.push_back(base);

  const auto base_up = base + perp;
  if (edge_between_gps(r, { base.x, base.y }, { base_up.x, base_up.y }) == entt::null)
    tiles.push_back(base_up);

  const auto base_down = base - perp;
  if (edge_between_gps(r, { base.x, base.y }, { base_down.x, base_down.y }) == entt::null)
    tiles.push_back(base_down);

  const auto a = get_tiles_in_line(r, map_c, base - axis, axis, depth);
  const auto b = get_tiles_in_line(r, map_c, base_up - axis, axis, depth);
  const auto c = get_tiles_in_line(r, map_c, base_down - axis, axis, depth);
  tiles.insert(tiles.end(), a.begin(), a.end());
  tiles.insert(tiles.end(), b.begin(), b.end());
  tiles.insert(tiles.end(), c.begin(), c.end());

  return tiles;
};

entt::entity
get_equipped_gun(entt::registry& r, const entt::entity e)
{
  const auto& body = r.get<DefaultBody>(e);
  const auto gun_e = get_slot_type(r, body.body, InventorySlotType::weapon);
  const auto gun_c = r.get<InventorySlotComponent>(gun_e);
  return gun_c.item_e;
};

int
get_damage_for_item(entt::registry& r, const entt::entity item_e)
{
  const auto& item_key = r.get<ItemKey>(item_e);
  const auto item_c = find_item(r, item_key.key);

  if (item_c.combat.has_value())
    return item_c.combat->damage;

  return 0;
};

int
get_damage_for_equipped_item(entt::registry& r, const entt::entity e)
{
  const auto item_e = get_equipped_gun(r, e);

  if (item_e == entt::null)
    return 0;

  return get_damage_for_item(r, item_e);
};

} // namespace game2d