// your header
#include "systems/player_input.hpp"

// components
#include "components/physics.hpp"
#include "components/player.hpp"
#include "components/rendering.hpp"

// other game2d headers
#include "factories.hpp"

// other lib headers
#include <imgui.h>

void
game2d::update_player_input_system(entt::registry& registry, engine::Application& app)
{
  glm::ivec2 mouse_pos = app.get_input().get_mouse_pos();
  ImGui::Begin("Mouse", NULL, ImGuiWindowFlags_NoFocusOnAppearing);
  ImGui::Text("%i %i", mouse_pos.x, mouse_pos.y);
  ImGui::End();

  // Convert WASD to input
  {
    auto view = registry.view<Player, Velocity>();
    view.each([&registry, &app](const auto& player, auto& vel) {
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
  }

  // Action: LMB destroys player body
  {
    auto view = registry.view<PositionInt, Player>();
    view.each([&registry, &app](const auto entity, const auto& pos, const auto& player) {
      if (app.get_input().get_mouse_lmb_down()) {
        registry.remove_if_exists<Player>(entity);
        registry.remove_if_exists<Velocity>(entity);
        auto e = create_player(registry, { pos.x, pos.y });
      }
    });
  }
};