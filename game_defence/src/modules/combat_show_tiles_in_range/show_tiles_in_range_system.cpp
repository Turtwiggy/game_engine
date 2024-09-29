#include "modules/combat_show_tiles_in_range/show_tiles_in_range_system.hpp"

#include "actors/actor_helpers.hpp"
#include "engine/algorithm_astar_pathfinding/astar_helpers.hpp"
#include "engine/entt/entity_pool.hpp"
#include "engine/entt/helpers.hpp"
#include "engine/enum/enum_helpers.hpp"
#include "engine/events/helpers/keyboard.hpp"
#include "engine/imgui/helpers.hpp"
#include "engine/maths/grid.hpp"
#include "engine/maths/maths.hpp"
#include "engine/renderer/transform.hpp"
#include "engine/sprites/components.hpp"
#include "engine/sprites/helpers.hpp"
#include "modules/actor_player/components.hpp"
#include "modules/combat/components.hpp"
#include "modules/event_damage/event_damage_helpers.hpp"
#include "modules/events/events_components.hpp"
#include "modules/map/components.hpp"
#include "modules/raws/raws_components.hpp"
#include "modules/system_select_unit/select_unit_components.hpp"
#include "modules/ui_inventory/ui_inventory_components.hpp"
#include "modules/ui_inventory/ui_inventory_helpers.hpp"

#include <glm/fwd.hpp>

namespace game2d {

enum class RangeType
{
  knife = 0,
  pistol,
  shotgun,
  plunger,

  count,
};

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

//  X
// XOX
//  X
std::vector<glm::ivec2>
get_tiles_for_knife(entt::registry& r, const MapComponent& map_c, const glm::ivec2& pos)
{
  auto tiles = generate_accessible_areas(r, map_c, pos, 1);

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

static bool show_distance_check = false;
static auto show_range_type = RangeType::knife;
static auto range_types_str = engine::enum_class_to_vec_str<RangeType>();

int
get_damage_for_equipped_item(entt::registry& r, const entt::entity e)
{
  const auto& body = r.get<DefaultBody>(e);
  const auto gun_e = get_slot_type(r, body.body, InventorySlotType::gun);
  const auto gun_c = r.get<InventorySlotComponent>(gun_e);
  if (gun_c.item_e != entt::null) {
    const auto& item = r.get<Item>(gun_c.item_e);

    if (item.melee.has_value())
      return item.melee.value().damage;

    if (item.ranged.has_value())
      return item.ranged.value().damage; // or could be bullet in gun...
  }

  return 0;
}

void
update_show_tiles_in_range_system(entt::registry& r)
{
  const auto map_e = get_first<MapComponent>(r);
  if (map_e == entt::null)
    return;
  const auto& map_c = r.get<MapComponent>(map_e);

#if defined(_DEBUG)
  ImGui::Begin("ShowTile Mode");
  WomboComboIn combo_in(range_types_str);
  combo_in.label = "ShowTileMode";
  combo_in.current_index = static_cast<int>(show_range_type);
  WomboComboOut combo_out = draw_wombo_combo(combo_in);
  if (combo_in.current_index != combo_out.selected)
    show_range_type = static_cast<RangeType>(combo_out.selected);
  ImGui::End();
#endif

  static EntityPool pool;

  const auto& view = r.view<PlayerComponent, const SelectedComponent>();
  for (const auto& [e, player_c, selected_c] : view.each()) {

    const auto* input_c = r.try_get<InputComponent>(e);
    if (input_c == nullptr)
      continue;

    const auto grid_pos = get_grid_position(r, e);
    // ImGui::Text("you: %i %i", grid_pos.x, grid_pos.y);

    std::vector<glm::ivec2> tiles;
    if (show_range_type == RangeType::knife)
      tiles = get_tiles_for_knife(r, map_c, grid_pos);
    if (show_range_type == RangeType::pistol)
      tiles = get_tiles_in_line(r, map_c, grid_pos, { input_c->rx, input_c->ry }, 1);
    if (show_range_type == RangeType::plunger)
      tiles = get_tiles_in_line(r, map_c, grid_pos, { input_c->rx, input_c->ry }, 4);
    if (show_range_type == RangeType::shotgun)
      tiles = get_tiles_for_shotgun(r, map_c, grid_pos, { input_c->rx, input_c->ry });

    // debug tiles with squares
    //
    pool.update(r, tiles.size()); // note: bad for multiple players
    for (int i = 0; const auto& tile_gp : tiles) {
      // ImGui::Text("accessible: %i %i", tile_gp.x, tile_gp.y);

      const auto debug_e = pool.instances[i];
      const auto offset = glm::vec2{ map_c.tilesize / 2.0f, map_c.tilesize / 2.0f };
      const auto anypos = glm::vec2(tile_gp * map_c.tilesize) + offset;
      set_position(r, debug_e, anypos);

      i++;
    }

// HACK: trial dealing damage to entities on the tiles
#if defined(_DEBUG)

    const auto& input = get_first_component<SINGLE_InputComponent>(r);
    const auto& evts = get_first_component<SINGLE_Events>(r);

    if (get_key_down(input, SDL_SCANCODE_SPACE)) {
      fmt::println("sending damage event...");

      for (const auto& tile : tiles) {
        if (tile.x < 0 || tile.x > map_c.xmax || tile.y < 0 || tile.y > map_c.ymax)
          continue;

        const auto idx = engine::grid::grid_position_to_index(tile, map_c.xmax);
        for (const auto map_e : map_c.map[idx]) {
          DamageEvent evt;
          evt.from = e;
          evt.to = map_e;
          // evt.amount = get_damage_for_equipped_item(r, e);
          static engine::RandomState rnd(0);
          evt.amount = engine::rand_det_s(rnd.rng, 0, 10);
          evts.dispatcher->trigger(evt);
          evts.dispatcher->update();
        }
      }
    }

#endif

    break; // only first player
  }
}

} // namespace game2d