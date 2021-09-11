// your header
#include "systems/player_system.hpp"

// game headers
#include "constants.hpp"

// components
#include "components/player.hpp"
#include "components/position.hpp"

// engine headers
#include <engine/grid.hpp>
#include <imgui.h>

void
game2d::update_player_system(entt::registry& registry, engine::Application& app)
{
  auto view = registry.view<Position, const Player>();

  ImGui::Begin("grid entities");
  auto mouse_pos = app.get_input().get_mouse_pos();
  ImGui::Text("mouse %i %i", mouse_pos.x, mouse_pos.y);

  int grid_s = grid_size;
  view.each([&app, &mouse_pos, &grid_s](const auto entity, auto& pos, const auto& player) {
    auto grid_pos = grid::convert_world_space_to_grid_space(mouse_pos, grid_s);

    pos.x = grid_pos.x;
    pos.y = grid_pos.y;

    ImGui::Text("e %i %i", pos.x, pos.y);
  });

  ImGui::End();
};