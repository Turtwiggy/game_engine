// your header
#include "systems/player_input.hpp"

// components
#include "components/physics.hpp"
#include "components/player.hpp"
#include "components/rendering.hpp"
#include "components/singleton_renderer.hpp"

// other engine headers
#include "engine/grid.hpp"

// other lib headers
#include <glm/glm.hpp>
#include <imgui.h>

void
game2d::update_player_input_system(entt::registry& registry, engine::Application& app)
{
  const auto& ri = registry.ctx<SINGLETON_RendererInfo>();

  // process game events if the viewport says so
  if (!ri.viewport_process_events)
    return;

  glm::ivec2 imgui_mouse_pos = app.get_input().get_mouse_pos();
  glm::vec2 imgui_viewport_tl = ri.viewport_pos;

  // this is what sdl2 returns from mouse_pos, if we wern't using moving viewport
  // glm::ivec2 window_tl = app.get_window().get_position();
  // glm::ivec2 mouse_pos = im_mouse_pos - window_tl;
  glm::ivec2 mouse_pos = imgui_mouse_pos - glm::ivec2(imgui_viewport_tl.x, imgui_viewport_tl.y);

  // adjust pos to keep square visible while mouse is in 4 corners
  const int grid_size = 16;
  glm::vec2 grid_adjusted_mouse_pos = mouse_pos;
  grid_adjusted_mouse_pos.x += grid_size / 2.0f;
  grid_adjusted_mouse_pos.y += grid_size / 2.0f;

  // ImGui::Begin("Mouse", NULL, ImGuiWindowFlags_NoFocusOnAppearing);
  // ImGui::Text("mouse %f %f", mouse_pos.x, mouse_pos.y);
  // ImGui::Text("grid adj %f %f", grid_adjusted_mouse_pos.x, grid_adjusted_mouse_pos.y);

  {
    const auto& view = registry.view<Player, PositionIntComponent>();
    view.each([&registry, &app, &grid_adjusted_mouse_pos, &grid_size](const auto& player, auto& pos) {
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

      // TODO: do not set position directly
      pos.x += vx * grid_size;
      pos.y += vy * grid_size;
      // ImGui::Text("player %i %i", pos.x, pos.y);
      glm::ivec2 grid_slot = engine::grid::world_space_to_grid_space({ pos.x, pos.y }, grid_size);
      // ImGui::Text("player grid %i %i", grid_slot.x, grid_slot.y);

      // Action: Update player position with RMB
      if (app.get_input().get_mouse_rmb_down()) {
        glm::ivec2 grid_slot = engine::grid::world_space_to_grid_space(grid_adjusted_mouse_pos, grid_size);
        glm::ivec2 world_space = grid_slot * grid_size;
        pos.x = world_space.x;
        pos.y = world_space.y;
      }
    });
  }

  // Update cursor
  {
    const auto& view = registry.view<CursorComponent, PositionIntComponent>();
    view.each([&grid_adjusted_mouse_pos, &grid_size](const auto entity, const auto& c, auto& pos) {
      glm::ivec2 grid_slot = engine::grid::world_space_to_grid_space(grid_adjusted_mouse_pos, grid_size);
      // ImGui::Text("mouse grid %i %i", grid_slot.x, grid_slot.y);

      glm::ivec2 world_space = grid_slot * grid_size;
      pos.x = grid_slot.x * grid_size;
      pos.y = grid_slot.y * grid_size;
      // ImGui::Text("mouse clamped %i %i", world_space.x, world_space.y);
    });
  }

  // ImGui::End();
};

// // Action: spawn object with LMB
// if (app.get_input().get_mouse_lmb_down()) {
//   glm::vec2 world_pos = adjusted_mouse_pos - glm::vec2(pos.x, pos.y);
//   glm::ivec2 world_pos_clamped = engine::grid::world_space_to_clamped_world_space(world_pos, grid_size);
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
//     registry.emplace<Size>(r, grid_size, grid_size);
//     registry.emplace<Sprite>(r, sprite::type::EMPTY);
//     registry.emplace<ZIndex>(r, 0);
//   }
// }