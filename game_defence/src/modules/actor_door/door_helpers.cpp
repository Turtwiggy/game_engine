#include "door_helpers.hpp"

#include "engine/entt/helpers.hpp"
#include "engine/maths/grid.hpp"
#include "engine/renderer/transform.hpp"
#include "modules/map/components.hpp"
#include "modules/ui_inventory/ui_inventory_components.hpp"

namespace game2d {

template<typename View, typename Predicate>
std::vector<entt::entity>
find_all(View view, Predicate pred)
{
  std::vector<entt::entity> result;
  for (auto entity : view)
    if (pred(entity))
      result.push_back(entity);
  return result;
};

std::vector<entt::entity>
contains_mobs(entt::registry& r, const glm::ivec2 gp)
{
  const auto& map_c = get_first_component<MapComponent>(r);

  // this approach means the entity boarding needs to be a part of the map...
  // if (gp_out_of_bounds(gp, map_c.xmax, map_c.ymax))
  //   return {};
  // const auto idx = engine::grid::grid_position_to_index(gp, map_c.xmax);
  // const auto ents = map_c.map[idx];
  // std::vector<entt::entity> results;
  // for (const auto e : ents)
  //   if (auto* body_c = r.try_get<DefaultBody>(e))
  //     results.push_back(e);
  // return results;

  const auto view = r.view<const DefaultBody, const TransformComponent>();
  const auto results = find_all(view, [&](entt::entity e) {
    const auto& t_c = view.get<TransformComponent>(e);
    const auto pos = glm::vec2{ t_c.position.x, t_c.position.y };
    const auto gpos = engine::grid::worldspace_to_grid_space(pos, map_c.tilesize);
    return gpos == gp;
  });
  return results;
};

} // namespace game2d