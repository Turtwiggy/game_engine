#include "helpers.hpp"

#include "components.hpp"
#include "engine/entt/helpers.hpp"
#include "engine/lifecycle/components.hpp"
#include "modules/events/events_components.hpp"
#include "modules/resolve_collisions/resolve_collisions_helpers.hpp"
#include "modules/system_names/components.hpp"
#include "modules/ui_inventory/ui_inventory_components.hpp"

#include <SDL2/SDL_log.h>

namespace game2d {

std::optional<MapInfo>
get_entity_mapinfo(entt::registry& r, entt::entity e)
{
  const auto& map = get_first_component<MapComponent>(r);

  for (int i = 0; i < map.xmax * map.ymax; i++) {
    const auto& es = map.map[i];

    const auto it = std::find(es.begin(), es.end(), e);
    if (it == es.end())
      continue;

    const auto tile_idx = static_cast<int>(it - es.begin());

    MapInfo info;
    info.idx_in_map = i;
    info.idx_in_map_tile = tile_idx;
    return info;
  }

  SDL_Log("ERROR: entity requested not on map");
  return std::nullopt;
};

void
remove_entity_from_map(entt::registry& r, const MapInfo& info)
{
  auto& map = get_first_component<MapComponent>(r);
  auto& es = map.map[info.idx_in_map];

  // SDL_Log("%s", std::format("remove_entity_from_map(): idx{} tile{} old size: {}", info.idx_in_map, info.idx_in_map_tile,
  // es.size());

  es.erase(es.begin() + info.idx_in_map_tile);

  // SDL_Log("%s", std::format("remove_entity_from_map(): idx{} new size: {}", info.idx_in_map, es.size()).c_str());
};

void
add_entity_to_map(entt::registry& r, const entt::entity src_e, const int idx)
{
  auto& map = get_first_component<MapComponent>(r);

  std::vector<entt::entity>& ents = map.map[idx];

  // if (ents.size() > 0)
  //   SDL_Log("%s", std::format("add_entity_to_map(): moving to tile that contains an entity").c_str());

  ents.push_back(src_e);
};

bool
move_entity_on_map(entt::registry& r, const entt::entity src_e, const int dst_idx)
{
  auto& map = get_first_component<MapComponent>(r);
  auto& dead = get_first_component<SINGLE_EntityBinComponent>(r);

  const auto mapinfo_opt = get_entity_mapinfo(r, src_e);
  if (!mapinfo_opt.has_value()) {
    SDL_Log("move_entity_on_map(): src_e not on map");
    return false;
  }

  const auto& dst_es = map.map[dst_idx];

  // hard case: dst is not clear,
  // and contain anything,
  // and any number of it.
  bool contains_mob = false;
  for (const auto dst_e : dst_es) {
    const bool has_body = r.try_get<DefaultBody>(dst_e) != nullptr;
    contains_mob |= has_body;
  }

  // easy case: dst is clear.
  if (!contains_mob) {
    remove_entity_from_map(r, mapinfo_opt.value());
    add_entity_to_map(r, src_e, dst_idx);
    SDL_Log("Moving %s on map %i", r.get<NameComponent>(src_e).first_name.c_str(), dst_idx);

    GridPositionChangedEvent evt;
    evt.e = src_e;
    evt.dst_idx = dst_idx;

    const auto& evts = get_first_component<SINGLE_Events>(r);
    evts.dispatcher->trigger(evt);
    evts.dispatcher->update();

    return true;
  }

  return false;
};

} // namespace game2d