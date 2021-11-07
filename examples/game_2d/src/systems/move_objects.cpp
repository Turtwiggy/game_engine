// your header
#include "systems/move_objects.hpp"

// components
#include "modules/physics/components.hpp"
#include "modules/physics/helpers.hpp"
#include "modules/renderer/components.hpp"

// other lib headers
#include <imgui.h>
#include <iostream>

void
game2d::update_move_objects_system(entt::registry& registry, engine::Application& app, float dt)
{
  const auto Sign = [](int x) { return x == 0 ? 0 : (x > 0 ? 1 : -1); };

  ImGui::Begin("Debug move objects", NULL, ImGuiWindowFlags_NoFocusOnAppearing);

  // actors and solids never overlap,
  // and solids dont overlap with solids

  // Collect the collidable objects
  std::vector<entt::entity> actors;
  std::vector<PhysicsObject> actors_aabb;
  std::vector<entt::entity> solids;
  std::vector<PhysicsObject> solids_aabb;

  const auto& coll_view = registry.view<const CollidableComponent>();
  coll_view.each([&registry, &solids, &actors, &solids_aabb, &actors_aabb](const auto entity, const auto& col) {
    if (!registry.all_of<PositionIntComponent, SizeComponent>(entity))
      return;
    auto pos = registry.get<PositionIntComponent>(entity);
    const auto size = registry.get<SizeComponent>(entity);
    PhysicsObject aabb;
    aabb.ent_id = static_cast<uint32_t>(entity);
    aabb.x_tl = static_cast<int>(pos.x - size.w / 2.0f);
    aabb.y_tl = static_cast<int>(pos.y - size.h / 2.0f);
    aabb.w = size.w;
    aabb.h = size.h;
    if (col.type == PhysicsType::SOLID) {
      solids.push_back(entity);
      solids_aabb.push_back(aabb);
    } else if (col.type == PhysicsType::ACTOR) {
      actors.push_back(entity);
      actors_aabb.push_back(aabb);
    }
  });

  ImGui::Text("actors %i", actors.size());
  ImGui::Text("actors_aabb %i", actors_aabb.size());
  ImGui::Text("solids %i", solids.size());
  ImGui::Text("solids_aabb %i", solids_aabb.size());

  // move actors, but stop at solids
  for (int i = 0; i < actors.size(); i++) {
    const auto& actor_eid = actors[i];
    const auto& actor_aabb = actors_aabb[i];
    auto& pos = registry.get<PositionIntComponent>(actor_eid);

    int move_x = static_cast<int>(pos.dx);
    if (move_x != 0) {
      pos.dx -= move_x;
      int sign = Sign(move_x);
      PhysicsObject potential_aabb;
      while (move_x != 0) {
        potential_aabb.x_tl = actor_aabb.x_tl + sign;
        potential_aabb.y_tl = actor_aabb.y_tl;
        potential_aabb.w = actor_aabb.w;
        potential_aabb.h = actor_aabb.h;
        bool collision_with_solid = collides(potential_aabb, solids_aabb);
        if (collision_with_solid) // ah! collision, maybe actor-solid callback?
        {
          std::cout << "actor collided with solid" << std::endl;
          break;
        }
        pos.x += sign;
        move_x -= sign;
      }
    }

    //   pos.dy += vel.y * dt;
    int move_y = static_cast<int>(pos.dy);
    if (move_y != 0) {
      pos.dy -= move_y;
      int sign = Sign(move_y);
      PhysicsObject potential_aabb;
      while (move_y != 0) {
        potential_aabb.x_tl = actor_aabb.x_tl + sign;
        potential_aabb.y_tl = actor_aabb.y_tl;
        potential_aabb.w = actor_aabb.w;
        potential_aabb.h = actor_aabb.h;
        bool collision_with_solid = collides(potential_aabb, solids_aabb);
        if (collision_with_solid) // ah! collision, maybe actor-solid callback?
        {
          std::cout << "actor collided with solid" << std::endl;
          break;
        }
        pos.y += sign;
        move_y -= sign;
      }
    }

    ImGui::Text("actor: %i %i %f %f", pos.x, pos.y, pos.dx, pos.dy);
    // end actors
  }

  // TODO: try and move solids?

  ImGui::End();
};