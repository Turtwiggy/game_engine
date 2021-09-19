// your header
#include "systems/player_system.hpp"

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

  ImGui::Begin("Mouse Pos");
  ImGui::Text("mouse %i %i", mouse_pos.x, mouse_pos.y);
  ImGui::End();
};