// your header
#include "systems/player_input.hpp"

// components
#include "components/physics.hpp"
#include "components/player.hpp"
#include "components/rendering.hpp"

// other game2d headers
#include "factories.hpp"

// other engine headers
#include "engine/grid.hpp"

// other lib headers
#include <imgui.h>

void
game2d::update_player_input_system(entt::registry& registry, engine::Application& app)
{
  glm::ivec2 mouse_pos = app.get_input().get_mouse_pos();
  ImGui::Begin("Mouse", NULL, ImGuiWindowFlags_NoFocusOnAppearing);
  ImGui::Text("%i %i", mouse_pos.x, mouse_pos.y);

  const int grid_size = 24;

  // adjust pos to keep square visible while mouse is in 4 corners
  glm::vec2 adjusted_mouse_pos = mouse_pos;
  adjusted_mouse_pos.x += grid_size / 2.0f;
  adjusted_mouse_pos.y += grid_size / 2.0f;
  ImGui::Text("adjusted mouse %i %i", adjusted_mouse_pos.x, adjusted_mouse_pos.y);

  // Convert WASD to input
  {
    auto view = registry.view<Player, PositionInt>();
    view.each([&registry, &app, &grid_size](const auto& player, auto& pos) {
      int vx = 0;
      int vy = 0;

      // Limitation: currently all players would be updated with wasd.
      if (app.get_input().get_key_down(SDL_SCANCODE_S))
        vy = 1;
      else if (app.get_input().get_key_down(SDL_SCANCODE_W))
        vy = -1;
      if (app.get_input().get_key_down(SDL_SCANCODE_A))
        vx = -1;
      else if (app.get_input().get_key_down(SDL_SCANCODE_D))
        vx = 1;

      pos.x += vx * grid_size;
      pos.y += vy * grid_size;
      ImGui::Text("player %i %i", pos.x, pos.y);

      glm::ivec2 grid_slot = engine::grid::world_space_to_grid_space({ pos.x, pos.y }, grid_size);
      ImGui::Text("player grid %i %i", grid_slot.x, grid_slot.y);
    });
  }

  // Update cursor
  {
    auto view = registry.view<PlayerCursor, PositionInt, const Size>();
    view.each([&registry, &app, &adjusted_mouse_pos, &grid_size](
                const auto entity, const auto& c, auto& pos, const auto& size) {
      glm::ivec2 grid_slot = engine::grid::world_space_to_grid_space(adjusted_mouse_pos, grid_size);
      ImGui::Text("mouse grid %i %i", grid_slot.x, grid_slot.y);

      glm::ivec2 world_space = grid_slot * grid_size;
      pos.x = grid_slot.x * grid_size;
      pos.y = grid_slot.y * grid_size;
      ImGui::Text("mouse clamped %i %i", world_space.x, world_space.y);
    });
  }

  // Action: LMB shoot
  {
    auto view = registry.view<Player, PositionInt>();
    view.each(
      [&registry, &app, &adjusted_mouse_pos, &grid_size](const auto entity, const auto& player, const auto& pos) {
        if (app.get_input().get_mouse_lmb_down()) {
          glm::vec2 world_pos = adjusted_mouse_pos - glm::vec2(pos.x, pos.y);
          glm::ivec2 world_pos_clamped = engine::grid::world_space_to_clamped_world_space(world_pos, grid_size);

          glm::vec2 vel = glm::vec2(world_pos_clamped.x, world_pos_clamped.y);
          if (!(vel.x == 0.0f && vel.y == 0.0f))
            vel = glm::normalize(vel);

          float bullet_speed = 100.0f;
          vel *= bullet_speed;

          { // create bullet
            entt::entity r = registry.create();
            registry.emplace<Velocity>(r, vel.x, vel.y);
            registry.emplace<Colour>(r, 1.0f, 1.0f, 1.0f, 1.0f);
            registry.emplace<PositionInt>(r, pos.x, pos.y);
            registry.emplace<Size>(r, 24.0f, 24.0f);
            registry.emplace<Sprite>(r, sprite::type::SQUARE);
            registry.emplace<ZIndex>(r, 0);
          }
          //
        }
      });
  }

  ImGui::End();
};