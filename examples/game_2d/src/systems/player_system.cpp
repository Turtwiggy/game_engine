// your header
#include "systems/player_system.hpp"

// components
#include "components/hex_cell.hpp"
#include "components/player.hpp"
#include "components/position.hpp"

// engine headers
// #include "engine/grid.hpp"
#include "helpers/hex_grid.hpp"

// other lib headers
#include <imgui.h>

void
game2d::update_player_system(entt::registry& registry, engine::Application& app)
{
  glm::ivec2 screen_wh = app.get_window().get_size();
  glm::ivec2 mouse_pos = app.get_input().get_mouse_pos();
  glm::ivec2 offset = { screen_wh.x / 2, screen_wh.y / 2 };

  ImGui::Begin("Mouse", NULL, ImGuiWindowFlags_NoFocusOnAppearing);
  ImGui::Text("%i %i", mouse_pos.x, mouse_pos.y);
  ImGui::End();

  // auto view = registry.view<HexMesh>();
  // registry.destroy(view.begin(), view.end());

  const auto view = registry.view<Player, PositionInt, HexCoord>();
  view.each([&registry, &mouse_pos, &offset](const auto& player, auto& player_pos, auto& hex_coord) {
    glm::ivec3 ihex_pos = { hex_coord.x, hex_coord.y, hex_coord.z };
    glm::vec2 world_pos = convert_hex_pos_to_world_pos(ihex_pos, offset);
    player_pos.x = world_pos.x;
    player_pos.y = world_pos.y;
  });
};