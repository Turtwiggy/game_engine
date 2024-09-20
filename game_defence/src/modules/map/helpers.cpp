#include "helpers.hpp"

#include "components.hpp"
#include "engine/algorithm_astar_pathfinding/astar_helpers.hpp"
#include "engine/entt/helpers.hpp"
#include "engine/lifecycle/components.hpp"
#include "engine/maths/grid.hpp"
#include "modules/actor_lootbag/components.hpp"

#include <fmt/core.h>

namespace game2d {

std::optional<MapInfo>
get_entity_mapinfo(entt::registry& r, entt::entity e)
{
  const auto& map = get_first_component<MapComponent>(r);

  for (int i = 0; i < map.xmax * map.ymax; i++) {
    const std::vector<entt::entity>& es = map.map[i];

    const auto it = std::find(es.begin(), es.end(), e);
    if (it == es.end())
      continue;

    const auto tile_idx = static_cast<int>(it - es.begin());

    MapInfo info;
    info.idx_in_map = i;
    info.idx_in_map_tile = tile_idx;
    return info;
  }

  fmt::println("ERROR: entity requested not on map");
  return std::nullopt;
};

void
remove_entity_from_map(entt::registry& r, const MapInfo& info)
{
  auto& map = get_first_component<MapComponent>(r);
  auto& es = map.map[info.idx_in_map];

  // fmt::println("remove_entity_from_map(): idx{} tile{} old size: {}", info.idx_in_map, info.idx_in_map_tile, es.size());

  es.erase(es.begin() + info.idx_in_map_tile);

  // fmt::println("remove_entity_from_map(): idx{} new size: {}", info.idx_in_map, es.size());
};

void
add_entity_to_map(entt::registry& r, const entt::entity src_e, const int idx)
{
  auto& map = get_first_component<MapComponent>(r);

  std::vector<entt::entity>& ents = map.map[idx];

  // if (ents.size() > 0)
  //   fmt::println("add_entity_to_map(): moving to tile that contains an entity");

  ents.push_back(src_e);
};

bool
move_entity_on_map(entt::registry& r, const entt::entity src_e, const int dst_idx)
{
  auto& map = get_first_component<MapComponent>(r);
  auto& dead = get_first_component<SINGLE_EntityBinComponent>(r);

  const auto mapinfo_opt = get_entity_mapinfo(r, src_e);
  if (!mapinfo_opt.has_value()) {
    fmt::println("move_entity_on_map(): src_e not on map");
    return false;
  }

  const std::vector<entt::entity>& dst_es = map.map[dst_idx];

  // easy case: dst is clear.
  if (dst_es.size() == 0) {
    remove_entity_from_map(r, mapinfo_opt.value());
    add_entity_to_map(r, src_e, dst_idx);
    return true;
  }

  // hard case: dst is not clear,
  // and contain anything,
  // and any number of it.

  // gamerule: allow move on to (any number) an inventory
  //
  bool contains_all_inventories = true;
  for (const auto dst_e : dst_es) {
    if (const auto* lootbag = r.try_get<LootbagComponent>(dst_e))
      contains_all_inventories = true;
    else
      contains_all_inventories = false;
  }

  if (contains_all_inventories) {
    // fmt::println("dst_es contains all inventories.");

    // situation: there's an inventory on the floor,
    // and you might have an inventory on you.
    // dont process the pickup rules, just allow the move.
    remove_entity_from_map(r, mapinfo_opt.value());
    add_entity_to_map(r, src_e, dst_idx);

    return true;
  }

  return false;
};

std::vector<glm::ivec2>
generate_path(entt::registry& r, int src_idx, int dst_idx, const size_t limit)
{
  const auto& map = get_first_component<MapComponent>(r);
  const auto a = engine::grid::index_to_grid_position(src_idx, map.xmax, map.ymax);
  const auto b = engine::grid::index_to_grid_position(dst_idx, map.xmax, map.ymax);
  const auto path = generate_direct(r, { a.x, a.y }, { b.x, b.y });

  // make sure path.size() < limit
  // return +1, as usually the first path[0] is the element the entity is currently standing on
  if (path.size() > limit)
    return { path.begin(), path.begin() + limit + 1 };

  return path;
};

} // namespace game2d