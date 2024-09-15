#include "system.hpp"

#include "components.hpp"
#include "entt/helpers.hpp"
#include "maths/grid.hpp"
#include "modules/grid/components.hpp"
#include "physics/components.hpp"

namespace game2d {

void
update_actor_cover_system(entt::registry& r)
{
  const auto map_e = get_first<MapComponent>(r);
  if (map_e == entt::null)
    return;
  const auto& map_c = get_first_component<MapComponent>(r);

  // remove anything's cover state
  const auto reqs = r.view<InCoverComponent>();
  r.remove<InCoverComponent>(reqs.begin(), reqs.end());

  // set all neighbours as in cover
  // todo: take in to account rooms

  /*
  const auto view = r.view<const CoverComponent, const PhysicsBodyComponent>();
  for (const auto& [e, cover_c, pb_c] : view.each()) {
    const auto pos_glm = glm::vec2{ pb_c.body->GetPosition().x, pb_c.body->GetPosition().y };
    const auto gp = engine::grid::worldspace_to_grid_space(pos_glm, map_c.tilesize);

    const auto neighbours_idxs = engine::grid::get_neighbour_indicies_with_diagonals(gp.x, gp.y, map_c.xmax, map_c.ymax);
    for (const auto& [dir, idx] : neighbours_idxs) {
      for (const auto e : map_c.map[idx])
        r.emplace<InCoverComponent>(e);
    }
  }
  */

  //
}

} // namespace game2d