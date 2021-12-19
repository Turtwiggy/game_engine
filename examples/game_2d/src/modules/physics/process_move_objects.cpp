// your header
#include "modules/physics/process_move_objects.hpp"

// components
#include "modules/physics/components.hpp"
#include "modules/physics/helpers.hpp"
#include "modules/renderer/components.hpp"

// other lib headers
#include <imgui.h>

// c++ lib headers
#include <algorithm>
#include <iostream>
#include <vector>

namespace game2d {

void
CALLBACK_actor_solid_collision(entt::registry& registry, const CollisionInfo2D& info)
{
  // std::cout << "actor collided, normal: " << info.normal.x << " " << info.normal.y << std::endl;
  if (registry.all_of<VelocityComponent>(info.eid)) {
    auto& vel = registry.get<VelocityComponent>(info.eid);

    auto& normal = info.normal;
    if (glm::abs(normal.y) > glm::abs(normal.x)) {
      // on a y-axis normal?
      vel.y = 0.0f;
    } else if (glm::abs(normal.x) > glm::abs(normal.y)) {
      // on an x-axis normal?
      vel.x = 0.0f;
    }
    // std::cout << "x: " << normal.x << " y:" << normal.y << std::endl;
  }
};

void
do_nothing(entt::registry& registry, const CollisionInfo2D& eid){
  // std::cout << "actor being squished between solids!" << std::endl;
};

} // namespace game2d

void
game2d::update_move_objects_system(entt::registry& registry, engine::Application& app, float dt)
{
  std::function<void(entt::registry&, CollisionInfo2D&)> actor_hit_solid_callback = CALLBACK_actor_solid_collision;
  std::function<void(entt::registry&, CollisionInfo2D&)> actor_being_squish_callback = do_nothing;

  ImGui::Begin("Debug move objects", NULL, ImGuiWindowFlags_NoFocusOnAppearing);

  // actors and solids never overlap,
  // and solids dont overlap with solids

  // Collect the collidable objects
  std::vector<PhysicsObject> actors_aabb;
  std::vector<PhysicsObject> solids_aabb;
  const auto& coll_view =
    registry.view<const CollidableComponent, const PositionIntComponent, const PhysicsSizeComponent>();

  PhysicsObject aabb;
  coll_view.each([&registry, &solids_aabb, &actors_aabb, &aabb](
                   const auto entity, const auto& col, const auto& pos, const auto& size) {
    aabb.ent_id = static_cast<uint32_t>(entity);
    aabb.x_tl = static_cast<int>(pos.x - (size.w / 2.0f));
    aabb.y_tl = static_cast<int>(pos.y - (size.h / 2.0f));
    aabb.w = size.w;
    aabb.h = size.h;
    if (col.type == PhysicsType::SOLID) {
      solids_aabb.push_back(aabb);
    } else if (col.type == PhysicsType::ACTOR) {
      actors_aabb.push_back(aabb);
    }
  });
  ImGui::Text("actors_aabb %i", actors_aabb.size());
  ImGui::Text("solids_aabb %i", solids_aabb.size());

  // move actors, but stop at solids
  for (int i = 0; i < actors_aabb.size(); i++) {
    auto& actor_aabb = actors_aabb[i];
    const auto& actor_eid = static_cast<entt::entity>(actor_aabb.ent_id);
    auto& pos = registry.get<PositionIntComponent>(actor_eid);

    if (registry.all_of<VelocityComponent>(actor_eid)) {
      const auto& vel = registry.get<VelocityComponent>(actor_eid);
      pos.dx += vel.x * dt;
      pos.dy += vel.y * dt;
    }

    move_actors_dir(registry, COLLISION_AXIS::X, pos.x, pos.dx, actor_aabb, solids_aabb, actor_hit_solid_callback);
    move_actors_dir(registry, COLLISION_AXIS::Y, pos.y, pos.dy, actor_aabb, solids_aabb, actor_hit_solid_callback);

    ImGui::Text("actor: %i %i %f %f", pos.x, pos.y, pos.dx, pos.dy);
    // end actors
  }

  // move solids
  // A solid interactors with an actor by carrying or pushing
  // An actor is carried if it is riding a solid
  // An actor is pushed if the solid's movement results in them overlapping
  // Pushing takes priority over carrying
  for (int i = 0; i < solids_aabb.size(); i++) {
    auto& solid_aabb = solids_aabb[i];
    const auto& solid_eid = static_cast<entt::entity>(solid_aabb.ent_id);
    auto& pos = registry.get<PositionIntComponent>(solid_eid);
    auto& size = registry.get<PhysicsSizeComponent>(solid_eid);

    if (registry.all_of<VelocityComponent>(solid_eid)) {
      const auto& vel = registry.get<VelocityComponent>(solid_eid);
      pos.dx += vel.x * dt;
      pos.dy += vel.y * dt;
    }

    int move_x = static_cast<int>(pos.dx);
    int move_y = static_cast<int>(pos.dy);

    if (move_x != 0 || move_y != 0) {
      // Loop through every Actor in the Level, add it to the list
      // if actor.IsRiding(this);
      // std::vector<entt::entity> riding;

      // Make this solid non-collidable for actors,
      // so that actors moved by it do not get stuck on it
      solid_aabb.collidable = false;

      // do x-axis movement....
      if (move_x != 0) {
        pos.dx -= move_x;
        pos.x += move_x;
        solid_aabb.x_tl = static_cast<int>(pos.x - (size.w / 2.0f));
        solid_aabb.y_tl = static_cast<int>(pos.y - (size.h / 2.0f));
        solid_aabb.w = size.w;
        solid_aabb.h = size.h;

        if (move_x > 0) {
          for (auto& actor_aabb : actors_aabb) {
            if (collide(solid_aabb, actor_aabb)) {
              const auto actor_eid = static_cast<entt::entity>(actor_aabb.ent_id);
              if (registry.all_of<PositionIntComponent>(actor_eid)) {
                // push right: right - left
                float dx = (solid_aabb.x_tl + solid_aabb.w) - actor_aabb.x_tl;
                auto& pos = registry.get<PositionIntComponent>(actor_eid);
                move_actors_dir(
                  registry, COLLISION_AXIS::X, pos.x, dx, actor_aabb, solids_aabb, actor_being_squish_callback);
              }
              // } else if (riding.contains(actor)) {
              // // Carry right
              //   actor.MoveX(moveX, null);
              // }
            }
          }
        } else {
          for (auto& actor_aabb : actors_aabb) {
            if (collide(solid_aabb, actor_aabb)) {
              const auto actor_eid = static_cast<entt::entity>(actor_aabb.ent_id);
              if (registry.all_of<PositionIntComponent>(actor_eid)) {
                // push left: left - right
                float dx = solid_aabb.x_tl - (actor_aabb.x_tl + actor_aabb.w);
                auto& pos = registry.get<PositionIntComponent>(actor_eid);
                move_actors_dir(
                  registry, COLLISION_AXIS::X, pos.x, dx, actor_aabb, solids_aabb, actor_being_squish_callback);
              }
            }
          }
        }
      }

      // do y-axis movement....
      if (move_y != 0) {
        pos.dy -= move_y;
        pos.y += move_y;
        solid_aabb.x_tl = static_cast<int>(pos.x - (size.w / 2.0f));
        solid_aabb.y_tl = static_cast<int>(pos.y - (size.h / 2.0f));
        solid_aabb.w = size.w;
        solid_aabb.h = size.h;

        if (move_y > 0) {
          for (auto& actor_aabb : actors_aabb) {
            if (collide(solid_aabb, actor_aabb)) {
              const auto actor_eid = static_cast<entt::entity>(actor_aabb.ent_id);
              if (registry.all_of<PositionIntComponent>(actor_eid)) {
                // push bottom: bottom - top
                float dy = (solid_aabb.y_tl + solid_aabb.h) - actor_aabb.y_tl;
                auto& pos = registry.get<PositionIntComponent>(actor_eid);
                move_actors_dir(
                  registry, COLLISION_AXIS::Y, pos.y, dy, actor_aabb, solids_aabb, actor_being_squish_callback);
              }
            }
          }
        } else {
          for (auto& actor_aabb : actors_aabb) {
            if (collide(solid_aabb, actor_aabb)) {
              const auto actor_eid = static_cast<entt::entity>(actor_aabb.ent_id);
              if (registry.all_of<PositionIntComponent>(actor_eid)) {
                // push top: top - bottom
                float dy = solid_aabb.y_tl - (actor_aabb.y_tl + actor_aabb.h);
                auto& pos = registry.get<PositionIntComponent>(actor_eid);
                move_actors_dir(
                  registry, COLLISION_AXIS::Y, pos.y, dy, actor_aabb, solids_aabb, actor_being_squish_callback);
              }
            }
          }
        }
      }
    }

    // Re-enable collisions for this solid
    solid_aabb.collidable = true;
  }

  ImGui::End();
};