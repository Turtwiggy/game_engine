#include "system.hpp"

#include "actors/actor_helpers.hpp"
#include "engine/algorithm_astar_pathfinding/astar_components.hpp"
#include "engine/entt/helpers.hpp"
#include "engine/renderer/transform.hpp"
#include "modules/actor_player/components.hpp"
#include "modules/map/components.hpp"
#include "modules/renderer/components.hpp"
#include "modules/spaceship_designer/generation/rooms_random.hpp"
#include "modules/system_move_player_on_map/move_player_on_map_helpers.hpp"

namespace game2d {

bool
inside_ship(entt::registry& r, const entt::entity e)
{
  const auto& map_e = get_first<MapComponent>(r);
  const auto& map = get_first_component<MapComponent>(r);
  const auto gp = get_grid_position(r, e);
  const auto rooms = inside_room(r, gp);
  const bool inside_spaceship = rooms.size() > 0;
  return inside_spaceship;
};

void
update_move_player_on_map_system(entt::registry& r)
{
  const float tilesize = 50.0f;
  const auto& ri = get_first_component<SINGLE_RendererInfo>(r);

  //
  // avoid movement if hovering ui
  // move direct i.e. WASD movement around grid
  //
  if (ri.viewport_hovered) {
    const auto& view =
      r.view<const PlayerComponent, const TransformComponent, InputComponent>(entt::exclude<GeneratedPathComponent>);
    for (const auto& [e, player_c, transform_c, inp_c] : view.each()) {

      const auto& map_e = get_first<MapComponent>(r);
      if (map_e == entt::null)
        continue;
      const auto& map_c = r.get<MapComponent>(map_e);

      if (!inside_ship(r, e)) {
        inp_c.unprocessed_move_down = false;
        continue; // only move if onboard
      }

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
        wp.x + round_away_from_zero(inp_c.lx) * tilesize,
        wp.y + round_away_from_zero(inp_c.ly) * tilesize,
      };

      move_action_lerp_to_neighbour(r, e, move_position);
    }
  }

  //
  // If the player has a generated path component attached,
  // lerp from your current position to the destination position
  //

  ImGui::Begin("GeneratedPath");

  const auto& path_view = r.view<GeneratedPathComponent>();
  for (const auto& [e, path_c] : path_view.each()) {
    ImGui::Separator();
    for (int i = 0; const auto& p : path_c.path) {

      if (i > 0)
        ImGui::SameLine();

      ImGui::Text("%i %i", p.x, p.y);
      i++;
    }
  }

  ImGui::End();
}

} // namespace game2d