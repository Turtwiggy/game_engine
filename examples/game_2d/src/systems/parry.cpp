// your header
#include "systems/parry.hpp"

// components
#include "components/flash_colour.hpp"
#include "components/parry.hpp"
#include "components/player.hpp"
#include "modules/physics/components.hpp"
#include "modules/renderer/components.hpp"

// other engine headers
#include "engine/grid.hpp"

// other lib headers
#include <glm/glm.hpp>

void
game2d::update_parry_system(entt::registry& registry, engine::Application& app, float dt)
{
  const auto& ri = registry.ctx<SINGLETON_RendererInfo>();

  // dont process game events if the viewport says so
  if (!ri.viewport_process_events)
    return;

  // check distance
  int parry_distance = 75;
  float ball_speed = 50.0f;

  // Player-Bouncy interaction
  {
    // For each player...
    auto player_view = registry.view<const Player, const PositionIntComponent>();
    player_view.each([&registry, &app, &parry_distance, &ball_speed](const auto& player, const auto& p_pos) {
      // For each bouncy object...
      const auto& view = registry.view<VelocityComponent, const PositionIntComponent, const ParryComponent>();
      view.each([&registry, &app, &p_pos, &parry_distance, &ball_speed](
                  auto entity, auto& vel, const auto& b_pos, const auto& bouncy) {
        int xdist = glm::abs(b_pos.x - p_pos.x);
        int ydist = glm::abs(b_pos.y - p_pos.y);

        // check ball is approaching player
        bool approaching_x = (b_pos.x > p_pos.x && vel.x <= 0) || (b_pos.x < p_pos.x && vel.x > 0);
        bool approaching_y = (b_pos.y < p_pos.y && vel.y > 0) || (b_pos.y > p_pos.y && vel.y <= 0);

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

        if (registry.all_of<ColourComponent, FlashColourComponent>(entity)) {
          auto& colour = registry.get<ColourComponent>(entity);
          auto& flash = registry.get<FlashColourComponent>(entity);

          // Change ball colour if its interactable
          if (able_to_be_parried)
            colour.colour = flash.flash_colour;
          else
            colour.colour = flash.start_colour;
        }

        able_to_be_parried = false;
      });
    });
  }
}