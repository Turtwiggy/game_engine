// your header
#include "systems/move_objects.hpp"

// components
#include "modules/physics/components.hpp"
#include "modules/renderer/components.hpp"

// other lib headers
#include <imgui.h>

void
game2d::update_move_objects_system(entt::registry& registry, engine::Application& app, float dt)
{
  // INPROGRESS:
  // Implement this:
  // https://maddythorson.medium.com/celeste-and-towerfall-physics-d24bd2ae0fc5

  // Collect all the physics objects in the scene
  std::vector<PhysicsObject> solids;
  z std::vector<PhysicsObject> actors;

  PhysicsObject po;
  const auto& coll_view =
    registry
      .view<const VelocityComponent, const CollidableComponent, const PositionIntComponent, const SizeComponent>();
  coll_view.each(
    [&registry, &po](const auto entity, const auto& vel, const auto& col, const auto& pos, const auto& size) {
      po.ent_id = static_cast<uint32_t>(entity);
      po.x_tl = static_cast<int>(pos.x - size.w / 2.0f);
      po.y_tl = static_cast<int>(pos.y - size.h / 2.0f);
      po.w = size.w;
      po.h = size.h;
      if (coll.type == PhysicsType::SOLID)
        solids.push_back(po);
      else if (coll.type == PhysicsType::ACTOR)
        actors.push_back(po);
    });

  const auto Sign = [](int x) { return x == 0 ? 0 : (x > 0 ? 1 : -1); };

  // Try and move all the actors
  for (auto& actor_eid : actors) {
    auto pos = registry.get<PositionIntComponent>(actor_eid);
    auto vel = registry.get<VelocityComponent>(actor_eid);
    auto size = registry.get<SizeComponent>(actor_eid);

    pos.dx += vel.x * dt;
    int move_x = static_cast<int>(pos.dx);
    if (move_x != 0) {
      pos.dx -= move_x;
      int sign = Sign(move_x);

      while (move_x != 0) {
        bool collision = false;
        // Check if any collisions
        int check_pos_x = pos.x + sign;

        //       // Check if point is in any object
        //       for (const auto& solid : solids) {
        //         if (registry.all_of<SizeComponent, PositionIntComponent>(solid)) {
        //           const auto& position = registry.get<PositionIntComponent>(solid);
        //           const auto& size = registry.get<SizeComponent>(solid);

        //           bool gt_coll = check_pos_x > position.x - (size.w / 2.0f);
        //           bool lt_coll = check_pos_x < position.x + (size.w / 2.0f);
        //           collision |= gt_coll && lt_coll;
        //           if (collision)
        //             break;
        //         }
        //       }
        //       if (collision) // ah! collision
        //         break;
        //       pos.x += sign;
        //       move_x -= sign;
      }
    }

    //   pos.dy += vel.y * dt;
    //   int move_y = static_cast<int>(pos.dy);
    //   if (move_y != 0) {
    //     pos.dy -= move_y;
    //     int sign = Sign(move_y);

    //     while (move_y != 0) {
    //       bool collision = false;
    //       // if(!collide_at(solids, obj.pos.x + sign))
    //       if (collision) // ah! collision
    //         break;
    //       pos.y += sign;
    //       move_y -= sign;
    //     }
    //   }
    // });
  }

  // Try and move all the solids that need moving
  // TODO this
};