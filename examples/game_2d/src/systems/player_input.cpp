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
#include <iostream>

void
game2d::update_player_input_system(entt::registry& registry, engine::Application& app)
{
  const auto& ri = registry.ctx<SINGLETON_RendererInfo>();
  const int& GRID_SIZE = registry.ctx<SINGLETON_GridSize>().size_xy;

  // process game events if the viewport says so
  // if (!ri.viewport_process_events)
  //   return;

  // temp
  const auto& controllers = app.get_input().controllers;
  if (controllers.size() > 0) {
    SDL_GameController* controller_0 = controllers[0];

    // axis
    // float l_analogue_x = app.get_input().get_axis_dir(controller_0,
    // SDL_GameControllerAxis::SDL_CONTROLLER_AXIS_LEFTX); float l_analogue_y =
    // app.get_input().get_axis_dir(controller_0, SDL_GameControllerAxis::SDL_CONTROLLER_AXIS_LEFTY); float r_analogue_x
    // = app.get_input().get_axis_dir(controller_0, SDL_GameControllerAxis::SDL_CONTROLLER_AXIS_RIGHTX); float
    // r_analogue_y = app.get_input().get_axis_dir(controller_0, SDL_GameControllerAxis::SDL_CONTROLLER_AXIS_RIGHTY);
    // float l_trigger =
    //   app.get_input().get_axis_dir(controller_0, SDL_GameControllerAxis::SDL_CONTROLLER_AXIS_TRIGGERLEFT);
    // float r_trigger =
    //   app.get_input().get_axis_dir(controller_0, SDL_GameControllerAxis::SDL_CONTROLLER_AXIS_TRIGGERRIGHT);
    // ImGui::Text("L Analogue X %f", l_analogue_x);
    // ImGui::Text("L Analogue Y %f", l_analogue_y);
    // ImGui::Text("R Analogue X %f", r_analogue_x);
    // ImGui::Text("R Analogue Y %f", r_analogue_y);
    // ImGui::Text("L Trigger %f", l_trigger);
    // ImGui::Text("R Trigger %f", r_trigger);

    bool start_button =
      app.get_input().get_button_down(controller_0, SDL_GameControllerButton::SDL_CONTROLLER_BUTTON_START);
    bool a_pressed = app.get_input().get_button_down(controller_0, SDL_GameControllerButton::SDL_CONTROLLER_BUTTON_A);
    bool a_pressed_again =
      app.get_input().get_button_down(controller_0, SDL_GameControllerButton::SDL_CONTROLLER_BUTTON_A);
    bool b_pressed = app.get_input().get_button_down(controller_0, SDL_GameControllerButton::SDL_CONTROLLER_BUTTON_B);
    bool x_pressed = app.get_input().get_button_down(controller_0, SDL_GameControllerButton::SDL_CONTROLLER_BUTTON_X);
    bool y_pressed = app.get_input().get_button_down(controller_0, SDL_GameControllerButton::SDL_CONTROLLER_BUTTON_Y);

    if (start_button)
      printf("start pressed... \n");
    if (a_pressed)
      printf("a pressed... \n");
    if (a_pressed_again)
      printf("a pressed... 2nd... \n");
    if (b_pressed)
      printf("b pressed... \n");
    if (x_pressed)
      printf("x pressed... \n");
    if (y_pressed)
      printf("y pressed... \n");

    bool dpad_up =
      app.get_input().get_button_down(controller_0, SDL_GameControllerButton::SDL_CONTROLLER_BUTTON_DPAD_UP);
    bool dpad_down =
      app.get_input().get_button_down(controller_0, SDL_GameControllerButton::SDL_CONTROLLER_BUTTON_DPAD_DOWN);
    bool dpad_left =
      app.get_input().get_button_down(controller_0, SDL_GameControllerButton::SDL_CONTROLLER_BUTTON_DPAD_LEFT);
    bool dpad_right =
      app.get_input().get_button_down(controller_0, SDL_GameControllerButton::SDL_CONTROLLER_BUTTON_DPAD_RIGHT);
    if (dpad_up)
      printf("dpad_up pressed... \n");
    if (dpad_down)
      printf("dpad_down pressed... \n");
    if (dpad_left)
      printf("dpad_left pressed... \n");
    if (dpad_right)
      printf("dpad_right pressed... \n");
  }

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