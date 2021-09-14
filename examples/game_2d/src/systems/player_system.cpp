// your header
#include "systems/player_system.hpp"

// game headers
#include "constants.hpp"

// components
#include "components/player.hpp"
#include "components/position.hpp"

// engine headers
#include "engine/grid.hpp"

// other lib headers
#include <imgui.h>

void
game2d::update_player_system(entt::registry& registry, engine::Application& app)
{
  auto mouse_pos = app.get_input().get_mouse_pos();

  {
    auto view = registry.view<PositionInt, const Player>();

    ImGui::Begin("mouse on square grid");
    ImGui::Text("mouse %i %i", mouse_pos.x, mouse_pos.y);

    int grid_s = grid_size;
    view.each([&app, &mouse_pos, &grid_s](const auto entity, auto& pos, const auto& player) {
      auto grid_pos = grid::convert_world_space_to_grid_space(mouse_pos, grid_s);

      // pos.x = mouse_pos.x;
      // pos.y = mouse_pos.y;

      ImGui::Text("clamped mouse %i %i", pos.x, pos.y);
    });
  }

  ImGui::End();
};