// your header
#include "systems/prototype/parry.hpp"

// components
#include "components/parry.hpp"
#include "components/physics.hpp"
#include "components/player.hpp"
#include "components/rendering.hpp"

// other engine headers
#include "engine/grid.hpp"

// other lib headers
#include <glm/glm.hpp>
#include <imgui.h>

void
game2d::update_parry_system(entt::registry& registry, engine::Application& app, float dt)
{
  ImGui::Begin("Parry", NULL, ImGuiWindowFlags_NoFocusOnAppearing);

  // Player-Bouncy interaction
  {
    //
    // For each player...
    auto player_view = registry.view<const Player, const PositionInt>();
    player_view.each([&registry, &app](const auto& player, const auto& p_pos) {
      //
      // For each bouncy object...
      auto& view = registry.view<Velocity, Colour, const PositionInt, const Bouncy>();
      view.each([&app, &p_pos](auto& vel, auto& colour, const auto& b_pos, const auto& bouncy) {
        int xdist = glm::abs(b_pos.x - p_pos.x);
        int ydist = glm::abs(b_pos.y - p_pos.y);
        ImGui::Text("ball x %i y %i ", xdist, ydist);
        ImGui::Text("ball vel x %f y %f", vel.x, vel.y);

        // check ball is approaching player
        bool approaching_x = (b_pos.x > p_pos.x && vel.x <= 0) || (b_pos.x < p_pos.x && vel.x > 0);
        bool approaching_y = (b_pos.y < p_pos.y && vel.y > 0) || (b_pos.y > p_pos.y && vel.y <= 0);
        ImGui::Text("Approaching x %i y %i", approaching_x, approaching_y);

        // check distance
        int parry_distance = 75;
        float ball_speed = 50.0f;

        bool able_to_be_parried = xdist < parry_distance && ydist < parry_distance;
        able_to_be_parried &= approaching_x || approaching_y;

        // Interact with ball if its near
        if (app.get_input().get_mouse_lmb_down() && able_to_be_parried) {

          // send ball off in the normal direction away from player
          glm::vec2 distance = glm::vec2(p_pos.x, p_pos.y) - glm::vec2(b_pos.x, b_pos.y);
          glm::vec2 dir = -glm::normalize(distance);

          vel.x = dir.x * 100.0f;
          vel.y = dir.y * 100.0f;
        }

        // Change ball colour if its interactable
        if (able_to_be_parried)
          colour.colour = glm::vec4(1.0f, 0.0f, 0.0f, 1.0f); // red
        else
          colour.colour = glm::vec4(1.0f); // white

        able_to_be_parried = false;
      });
    });
  }

  ImGui::End();
}