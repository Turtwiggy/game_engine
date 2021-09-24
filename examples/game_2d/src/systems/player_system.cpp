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
  //
  auto pos = app.get_input().get_mouse_pos();
  ImGui::Begin("Mouse", NULL, ImGuiWindowFlags_NoFocusOnAppearing);
  ImGui::Text("%i %i", pos.x, pos.y);
  ImGui::End();
};