#include "trait_pull_helpers.hpp"

#include "actors/actor_helpers.hpp"
#include "engine/algorithm_astar_pathfinding/astar_helpers.hpp"
#include "engine/maths/grid.hpp"

namespace game2d {

void
handle_damage_event_apply_pull(entt::registry& r, const DamageEvent& evt)
{
  const auto from_e = evt.from;
  const auto to_e = evt.to;

  if (from_e == entt::null)
    return; // probably a system damage event

  for (const Trait& trait : evt.traits) {
    if (trait.key.find("pull") == std::string::npos)
      continue;
    SDL_Log("Applying pull effect");

    const auto gp_a = get_grid_position(r, from_e);
    const auto gp_b = get_grid_position(r, to_e);

    // direction to push enemy
    glm::ivec2 pull_dir = gp_b - gp_a;
    if (pull_dir.x != 0)
      pull_dir.x /= -glm::abs(pull_dir.x);
    if (pull_dir.y != 0)
      pull_dir.y /= -glm::abs(pull_dir.y);

    const auto dst_gp = gp_b + pull_dir;
    const auto path = generate_direct_with_diagonals(r, { gp_b.x, gp_b.y }, { dst_gp.x, dst_gp.y });
    if (path.size() > 0) {
      const auto& map_c = get_first_component<MapComponent>(r);

      GeneratedPathComponent path_c;
      path_c.path = path;
      path_c.path_cleared.resize(path.size(), false);
      path_c.src_pos = get_position(r, to_e);
      path_c.dst_pos = engine::grid::grid_space_to_world_space_center(path[path.size() - 1], map_c.tilesize);

      r.emplace_or_replace<GeneratedPathComponent>(to_e, path_c);
    }
  }
}

} // namespace game2d