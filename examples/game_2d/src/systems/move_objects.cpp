// your header
#include "systems/move_objects.hpp"

// components
#include "modules/physics/components.hpp"
#include "modules/renderer/components.hpp"

// other lib headers
#include <imgui.h>

// Move an Actor
void
game2d::update_move_objects_system(entt::registry& registry, engine::Application& app, float dt)
{
  const auto& view = registry.view<VelocityComponent, PositionIntComponent>();
  view.each([&registry, &dt](const auto& vel, auto& pos) {
    const auto Sign = [](int x) { return x == 0 ? 0 : (x > 0 ? 1 : -1); };

    pos.dx += vel.x * dt;
    int move_x = static_cast<int>(pos.dx);

    if (move_x != 0) {
      pos.dx -= move_x;
      int sign = Sign(move_x);

      while (move_x != 0) {
        // if(!collide_at(solids, obj.pos.x + sign))
        if (true) {
          // there is no solid beside
          pos.x += sign;
          move_x -= sign;
        } else {
          // there is a solid beside
          break;
        }
      }
    }

    pos.dy += vel.y * dt;
    int move_y = static_cast<int>(pos.dy);

    if (move_y != 0) {
      pos.dy -= move_y;
      int sign = Sign(move_y);

      while (move_y != 0) {
        // if(!collide_at(solids, obj.pos.x + sign))
        if (true) {
          // there is no solid beside
          pos.y += sign;
          move_y -= sign;
        } else {
          // there is a solid beside
          break;
        }
      }
    }

    //
  });
};