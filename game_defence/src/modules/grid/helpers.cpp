#include "helpers.hpp"

#include "actors/helpers.hpp"
#include "components.hpp"
#include "entt/helpers.hpp"
#include "lifecycle/components.hpp"
#include "maths/grid.hpp"
#include "modules/algorithm_astar_pathfinding/helpers.hpp"
#include "modules/items_pickup/components.hpp"
#include "modules/ui_inventory/components.hpp"
#include "modules/ui_inventory/helpers.hpp"

#include <fmt/core.h>

namespace game2d {

void
add_entity_to_map(entt::registry& r, const entt::entity src_e, const int idx)
{
  auto& map = get_first_component<MapComponent>(r);

  if (map.map[idx] != entt::null) {
    fmt::println("add_entity_to_map(): src contains an entity");
    return;
  }

  map.map[idx] = src_e;
};

bool
move_entity_on_map(entt::registry& r, const int idx_a, const int idx_b)
{
  auto& map = get_first_component<MapComponent>(r);
  auto& dead = get_first_component<SINGLETON_EntityBinComponent>(r);

  const entt::entity src_e = map.map[idx_a];
  const entt::entity dst_e = map.map[idx_b];

  if (src_e == entt::null) {
    fmt::println("move_entity_on_map(): src does not contain entity");
    return false;
  }

  if (dst_e != entt::null) {
    fmt::println("move_entity_on_map(): dst not clear for move");

    if (auto* pickup = r.try_get<AbleToBePickedUp>(dst_e)) {
      fmt::println("move_entity_on_map(): dst is an item");

      // Add it to the inventory of the thing being picked up if there is space
      if (auto* inv = r.try_get<DefaultInventory>(src_e)) {

        bool item_added = false;

        // add scrap to first free slot
        for (size_t i = 0; i < inv->inv.size(); i++) {
          const auto slot_e = inv->inv[i];
          auto& slot_c = r.get<InventorySlotComponent>(slot_e);

          if (slot_c.item_e != entt::null)
            continue; // slot not free

          const auto& item_type = r.get<ItemTypeComponent>(dst_e);
          if (item_type.type == ItemType::scrap) {
            slot_c.item_e = create_inv_scrap(r, slot_e);
            item_added = true;
          }

          break; // item is not scrap?
        }

        if (!item_added) {
          fmt::println("warning: would pickup item but inv full; not moving");
          return false;
        }

        //
      } else {
        fmt::println("warning: no inventory to pickup; not moving");
        return false;
      }

      fmt::println("item picked up");

      // remove that item.
      dead.dead.emplace(dst_e);

      map.map[idx_a] = entt::null;
      map.map[idx_b] = src_e;
      return true;
    }

    return false;
  }

  map.map[idx_a] = entt::null;
  map.map[idx_b] = src_e;
  return true;
};

std::vector<glm::ivec2>
generate_path(entt::registry& r, const entt::entity src_e, const glm::ivec2& worldspace_pos, const size_t limit)
{
  const auto& map = get_first_component<MapComponent>(r);

  const auto src_idx = engine::grid::worldspace_to_index(get_position(r, src_e), map.tilesize, map.xmax, map.ymax);
  const auto dst_idx = engine::grid::worldspace_to_index(worldspace_pos, map.tilesize, map.xmax, map.ymax);

  auto path = generate_direct(r, map, src_idx, dst_idx, map.edges);

  // make sure path.size() < limit
  // return +1, as usually the first path[0] is the element the entity is currently standing on
  if (path.size() > limit)
    return { path.begin(), path.begin() + limit + 1 };

  return path;
};

} // namespace game2d