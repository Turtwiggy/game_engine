// your header
#include "systems/player_input_system.hpp"

// components
#include "components/physics.hpp"
#include "components/player.hpp"

// other lib headers
#include <imgui.h>

void
game2d::update_player_input_system(entt::registry& registry, engine::Application& app)
{
  float delta_time = app.get_delta_time();
  // glm::ivec2 screen_wh = app.get_window().get_size();
  // glm::ivec2 screen_center = { screen_wh.x / 2, screen_wh.y / 2 };
  glm::ivec2 mouse_pos = app.get_input().get_mouse_pos();
  ImGui::Begin("Mouse", NULL, ImGuiWindowFlags_NoFocusOnAppearing);
  ImGui::Text("%i %i", mouse_pos.x, mouse_pos.y);
  ImGui::End();

  const auto view = registry.view<Player, Velocity>();
  view.each([&registry, &delta_time, &app](const auto& player, auto& vel) {
    int vx = 0;
    int vy = 0;

    // Limitation: currently all players would be updated with wasd.

    if (app.get_input().get_key_held(SDL_SCANCODE_S))
      vy = 1;
    else if (app.get_input().get_key_held(SDL_SCANCODE_W))
      vy = -1;
    if (app.get_input().get_key_held(SDL_SCANCODE_A))
      vx = -1;
    else if (app.get_input().get_key_held(SDL_SCANCODE_D))
      vx = 1;

    float speed = 100.0f;
    vel.x = vx * speed;
    vel.y = vy * speed;
  });
};