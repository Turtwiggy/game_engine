#include "system.hpp"

#include "entt/helpers.hpp"
#include "lifecycle/components.hpp"
#include "modules/grid/components.hpp"

#include "fmt/core.h"

#include <algorithm>

namespace game2d {

void
remove_dead_entities(entt::registry& r)
{
  auto& map = get_first_component<MapComponent>(r);

  // if something was killed, remove it from the map
  const auto& dead = get_first_component<SINGLETON_EntityBinComponent>(r);

  for (const auto& e : dead.dead) {
    const auto it = std::find(map.map.begin(), map.map.end(), e);
    if (it == map.map.end()) {
      // fmt::println("Map: dead entity was not in map?");
      // this case occurs when things like bullets or anything else dies
      return;
    }
    fmt::println("Map: dead entity was in map");
    const auto idx = it - map.map.begin();
    map.map[idx] = entt::null;
  }
};

void
update_pathfinding_system(entt::registry& r, const float& dt)
{
  const float dt_ms = dt * 1000.0f;

  if (get_first<MapComponent>(r) == entt::null)
    return;

  remove_dead_entities(r);
}

} // namespace game2d