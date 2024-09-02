#include "helpers.hpp"

#include "entt/helpers.hpp"
#include "maths/grid.hpp"
#include "modules/grid/components.hpp"
#include "modules/grid/helpers.hpp"
#include "modules/system_move_to_target_via_lerp/components.hpp"
#include "modules/system_turnbased/components.hpp"

namespace game2d {

void
move_action_common(entt::registry& r, const entt::entity e, const glm::vec2& dst_wp)
{
  const auto& map = get_first_component<MapComponent>(r);

  const auto limit = r.get<MoveLimitComponent>(e).amount;
  const auto path = generate_path(r, e, dst_wp, limit);
  if (path.size() < 2)
    return;

  const auto next_dst = path[1];
  const auto next_idx = engine::grid::grid_position_to_index(next_dst, map.xmax);

  // move action...
  const auto it = std::find(map.map.begin(), map.map.end(), e);
  const auto idx = static_cast<int>(it - map.map.begin());
  const int a = static_cast<int>(idx);
  const int b = next_idx;

  const bool moved = move_entity_on_map(r, a, b);
  if (!moved)
    return;

  // Lerp the player model, independent of the grid representation
  const auto offset = glm::vec2{ map.tilesize / 2.0f, map.tilesize / 2.0f };
  remove_if_exists<LerpingToTarget>(r, e);
  LerpingToTarget lerp;
  lerp.a = engine::grid::index_to_world_position(a, map.xmax, map.ymax, map.tilesize) + offset;
  lerp.b = engine::grid::index_to_world_position(b, map.xmax, map.ymax, map.tilesize) + offset;
  lerp.t = 0.0f;
  r.emplace<LerpingToTarget>(e, lerp);
};

} // namespace game2d