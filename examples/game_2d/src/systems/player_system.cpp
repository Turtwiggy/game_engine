// your header
#include "systems/player_system.hpp"

// components
#include "components/player.hpp"
#include "components/rendering.hpp"

// engine headers
#include "helpers/hex_grid.hpp"

// other lib headers
#include <imgui.h>

void
game2d::update_player_system(entt::registry& registry, engine::Application& app)
{
  glm::ivec2 screen_wh = app.get_window().get_size();
  glm::ivec2 screen_center = { screen_wh.x / 2, screen_wh.y / 2 };
  glm::ivec2 mouse_pos = app.get_input().get_mouse_pos();

  ImGui::Begin("Mouse", NULL, ImGuiWindowFlags_NoFocusOnAppearing);
  ImGui::Text("%i %i", mouse_pos.x, mouse_pos.y);
  ImGui::End();

  // Set player to hex grid pos
  // const auto view = registry.view<Player, PositionInt, HexCoord>();
  // view.each([&registry, &mouse_pos, &screen_center](const auto& player, auto& player_pos, auto& hex_coord) {
  //   glm::ivec3 ihex_pos = { hex_coord.x, hex_coord.y, hex_coord.z };
  //   glm::vec2 world_pos = convert_hex_pos_to_world_pos(ihex_pos, screen_center);
  //   ImGui::Text("%f %f", world_pos.x, world_pos.y);
  //   player_pos.x = world_pos.x;
  //   player_pos.y = world_pos.y;
  // });

  // Set player to mouse
  const auto view = registry.view<Player, PositionInt>();
  view.each([&registry, &mouse_pos](const auto& player, auto& player_pos) {
    player_pos.x = mouse_pos.x;
    player_pos.y = mouse_pos.y;
  });
};