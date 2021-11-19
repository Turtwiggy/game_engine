// your header
#include "systems/player_input.hpp"

// components
#include "components/player.hpp"
#include "components/singleton_grid.hpp"
#include "modules/physics/components.hpp"
#include "modules/renderer/components.hpp"

// other engine headers
#include "engine/grid.hpp"

// other lib headers
#include <glm/glm.hpp>
#include <imgui.h>

void
game2d::update_player_input_system(entt::registry& registry, engine::Application& app)
{
  const auto& ri = registry.ctx<SINGLETON_RendererInfo>();
  const int& GRID_SIZE = registry.ctx<SINGLETON_GridSize>().size_xy;

  // process game events if the viewport says so
  // if (!ri.viewport_process_events)
  //   return;

  glm::ivec2 imgui_mouse_pos = app.get_input().get_mouse_pos();
  glm::vec2 imgui_viewport_tl = ri.viewport_pos;
  glm::ivec2 mouse_pos = imgui_mouse_pos - glm::ivec2(imgui_viewport_tl.x, imgui_viewport_tl.y);
  glm::vec2 mouse_pos_adjusted_in_worldspace = mouse_pos;
  mouse_pos_adjusted_in_worldspace.x += GRID_SIZE / 2.0f;
  mouse_pos_adjusted_in_worldspace.y += GRID_SIZE / 2.0f;

  {
    const auto& view = registry.view<Player, PositionIntComponent>();
    view.each([&registry, &app, &mouse_pos_adjusted_in_worldspace, &GRID_SIZE](const auto& player, auto& pos) {
      // Action: Move, Convert WASD to input
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

      // move in grid sizes
      pos.dx += vx * GRID_SIZE;
      pos.dy += vy * GRID_SIZE;
      // ImGui::Text("player %i %i %f %f", pos.x, pos.y, pos.dx, pos.dy);

      glm::ivec2 grid_slot = engine::grid::world_space_to_grid_space({ pos.x, pos.y }, GRID_SIZE);
      // ImGui::Text("player grid %i %i", grid_slot.x, grid_slot.y);

      // Action: Update player position with RMB
      if (app.get_input().get_mouse_rmb_down()) {
        glm::ivec2 grid_slot = engine::grid::world_space_to_grid_space(mouse_pos_adjusted_in_worldspace, GRID_SIZE);
        glm::ivec2 world_space = grid_slot * GRID_SIZE;
        pos.x = world_space.x;
        pos.y = world_space.y;
      }
    });
  }

  // Update cursor
  {
    const auto& view = registry.view<CursorComponent, PositionIntComponent>();
    view.each([&mouse_pos_adjusted_in_worldspace, &GRID_SIZE](const auto entity, const auto& c, auto& pos) {
      glm::ivec2 grid_slot = engine::grid::world_space_to_grid_space(mouse_pos_adjusted_in_worldspace, GRID_SIZE);
      // ImGui::Text("mouse grid %i %i", grid_slot.x, grid_slot.y);

      glm::ivec2 world_space = grid_slot * GRID_SIZE;
      pos.x = grid_slot.x * GRID_SIZE;
      pos.y = grid_slot.y * GRID_SIZE;
      // ImGui::Text("mouse clamped %i %i", world_space.x, world_space.y);
    });
  }

  // ImGui::End();
};

// // Action: spawn object with LMB
// if (app.get_input().get_mouse_lmb_down()) {
//   glm::vec2 world_pos = adjusted_mouse_pos - glm::vec2(pos.x, pos.y);
//   glm::ivec2 world_pos_clamped = engine::grid::world_space_to_clamped_world_space(world_pos, GRID_SIZE);
//   glm::vec2 vel = glm::vec2(world_pos_clamped.x, world_pos_clamped.y);
//   if (!(vel.x == 0.0f && vel.y == 0.0f))
//     vel = glm::normalize(vel);
//   float bullet_speed = 100.0f;
//   vel *= bullet_speed;
//   {
//     entt::entity r = registry.create();
//     registry.emplace<Velocity>(r, vel.x, vel.y);
//     registry.emplace<Colour>(r, 1.0f, 1.0f, 1.0f, 1.0f);
//     registry.emplace<PositionInt>(r, pos.x, pos.y);
//     registry.emplace<Size>(r, GRID_SIZE, GRID_SIZE);
//     registry.emplace<Sprite>(r, sprite::type::EMPTY);
//     registry.emplace<ZIndex>(r, 0);
//   }
// }