// your header
#include "systems/move_objects.hpp"

// components
#include "components/physics.hpp"
#include "components/rendering.hpp"

// other lib headers
#include <imgui.h>

void
game2d::update_move_objects_system(entt::registry& registry, engine::Application& app)
{
  float delta_time = app.get_delta_time();

  const auto view = registry.view<Velocity, PositionInt>();
  view.each([&registry, &delta_time](const auto& vel, auto& pos) {
    const auto Sign = [](int x) { return x == 0 ? 0 : (x > 0 ? 1 : -1); };

    pos.dx += vel.x * delta_time;
    int move_x = static_cast<int>(pos.dx);
    if (move_x != 0) {
      pos.dx -= move_x;
      int sign = Sign(move_x);
      while (move_x != 0) {
        bool collision = false;
        // if(!collide_at(solids, obj.pos.x + sign))
        if (!collision) {
          // there is no solid beside
          pos.x += sign;
          move_x -= sign;
        } else {
          // there is a solid beside
          // collide();
          break;
        }
      }
    }

    pos.dy += vel.y * delta_time;
    int move_y = static_cast<int>(pos.dy);
    if (move_y != 0) {
      pos.dy -= move_y;
      int sign = Sign(move_y);
      while (move_y != 0) {
        bool collision = false;
        // if(!collide_at(solids, obj.pos.x + sign))
        if (!collision) {
          // there is no solid beside
          pos.y += sign;
          move_y -= sign;
        } else {
          // there is a solid beside
          // collide();
          break;
        }
      }
    }

    //
  });
};