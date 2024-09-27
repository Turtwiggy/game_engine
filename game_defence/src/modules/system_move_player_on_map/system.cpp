#include "system.hpp"

#include "actors/actor_helpers.hpp"
#include "engine/entt/helpers.hpp"
#include "engine/renderer/transform.hpp"
#include "modules/actor_player/components.hpp"
#include "modules/map/components.hpp"
#include "modules/spaceship_designer/generation/components.hpp"
#include "modules/spaceship_designer/generation/rooms_random.hpp"
#include "modules/system_move_player_on_map/helpers.hpp"

namespace game2d {

bool
inside_ship(entt::registry& r, const entt::entity e)
{
  const auto& map_e = get_first<MapComponent>(r);
  const auto& map = get_first_component<MapComponent>(r);
  const auto& results_e = get_first<DungeonGenerationResults>(r);
  const auto& results = r.get<DungeonGenerationResults>(results_e);
  const auto gp = get_grid_position(r, e);
  const auto rooms = inside_room(r, gp);
  const bool inside_spaceship = rooms.size() > 0;
  return inside_spaceship;
};

void
update_move_player_on_map_system(entt::registry& r)
{
  const auto& map_e = get_first<MapComponent>(r);
  if (map_e == entt::null)
    return;
  const auto& map_c = r.get<MapComponent>(map_e);

  const auto& view = r.view<const PlayerComponent, InputComponent, TransformComponent>();
  for (const auto& [e, player_c, inp_c, transform_c] : view.each()) {
    if (!inside_ship(r, e))
      continue; // only move if onboard

    if (!inp_c.unprocessed_move_down)
      continue; // no input pressed
    inp_c.unprocessed_move_down = false;

    const auto round_away_from_zero = [](const float value) -> float {
      if (value > 0.0f)
        return std::ceil(value);
      else if (value < 0.0f)
        return std::floor(value);
      else
        return 0.0f;
    };

    const auto wp = glm::vec2{ transform_c.position.x, transform_c.position.y };

    // do the move
    const auto move_position = glm::vec2{
      wp.x + round_away_from_zero(inp_c.lx) * float(map_c.tilesize),
      wp.y + round_away_from_zero(inp_c.ly) * float(map_c.tilesize),
    };

    move_action_common(r, e, move_position);
  }
}

} // namespace game2d