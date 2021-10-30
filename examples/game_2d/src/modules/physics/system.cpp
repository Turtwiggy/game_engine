// your header
#include "system.hpp"

// game2d
#include "modules/physics/components.hpp"
#include "modules/physics/helpers.hpp"
#include "modules/renderer/components.hpp"

// other lib headers
#include <glm/glm.hpp>
#include <imgui.h>

void
game2d::init_physics_system(entt::registry& registry)
{
  registry.set<SINGLETON_PhysicsComponent>(SINGLETON_PhysicsComponent());
};

void
game2d::update_physics_system(entt::registry& registry, engine::Application& app, float dt)
{
  SINGLETON_PhysicsComponent& p = registry.ctx<SINGLETON_PhysicsComponent>();

  ImGui::Begin("Physics Debug");
  {
    p.collidable.clear();
    const auto& view = registry.view<const CollidableComponent, const PositionIntComponent, const SizeComponent>();
    view.each([&p](const auto entity, const auto& col, const auto& pos, const auto& size) {
      PhysicsObject po;
      po.ent_id = static_cast<uint32_t>(entity);
      po.x_tl = (int)(pos.x - size.w / 2.0f);
      po.y_tl = (int)(pos.y - size.h / 2.0f);
      po.w = size.w;
      po.h = size.h;

      p.collidable.push_back(po);
    });
  }
  ImGui::Text("Physics objects %i", p.collidable.size());

  generate_filtered_broadphase_collisions(p.collidable, p.filtered_collisions);
  ImGui::Text("Collisions %i", p.filtered_collisions.size());

  //         // Check game logic!
  //         bool valid_collision = game_collision_matrix(new_obj.get().collision_layer,
  //         old_obj.get().collision_layer);

  // How to process filtered collision events?
  // for (auto& c : filtered_collisions) {
  //   //
  //   uint32_t id_0 = c.second.ent_id_0;
  //   uint32_t id_1 = c.second.ent_id_1;
  //   //       // Find the objs in the read-only list
  //   //       auto& obj_0_it = std::find_if(
  //   //         collidable.begin(), collidable.end(), [&id_0](const auto& obj) { return obj.get().id == id_0; });
  //   //       auto& obj_1_it = std::find_if(
  //   //         collidable.begin(), collidable.end(), [&id_1](const auto& obj) { return obj.get().id == id_1; });
  //   //       if (obj_0_it == collidable.end() || obj_1_it == collidable.end()) {
  //   //         std::cerr << "Collision entity not in entity list" << std::endl;
  //   //         continue;
  //   //       }
  //   //       CollisionEvent eve(obj_0_it->get(), obj_1_it->get());
  //   //       gs.collision_events.push_back(eve);

  // bool
  // game_collision_matrix(CollisionLayer& y_l1, CollisionLayer& x_l2)
  // {
  //   int x_max = static_cast<int>(CollisionLayer::Count);
  //   int y = static_cast<int>(y_l1); // c1 is 0, 1, or 2
  //   int x = static_cast<int>(x_l2); // c2 is 0, 1, or 2
  //   // collision_matrix is a global (non-mutable) var
  //   bool val = grid::get_cell_mirrored_grid(GAME_COLL_MATRIX, x, y, x_max);
  //   return val;
  // }

  ImGui::End();
};