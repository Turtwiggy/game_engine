// your header
#include "systems/process_physics.hpp"

// components
#include "modules/physics/components.hpp"

// helpers
#include "helpers/physics_layers.hpp"

// other lib headers
#include <imgui.h>
#include <iostream>

void
game2d::update_process_physics_system(entt::registry& registry, engine::Application& app, float dt)
{
  SINGLETON_PhysicsComponent& p = registry.ctx<SINGLETON_PhysicsComponent>();

  ImGui::Begin("Game collision ui...", NULL, ImGuiWindowFlags_NoFocusOnAppearing);
  ImGui::Text("Collisions %i", p.frame_collisions.size());
  ImGui::Text("Persistent Collisions %i", p.persistent_collisions.size());

  for (const auto& coll : p.collision_enter) {
    // auto& e0_layer = registry.get<CollidableComponent>(static_cast<entt::entity>(coll.ent_id_0));
    // auto& e0_layer_name = std::string(magic_enum::enum_name(static_cast<GameCollisionLayer>(e0_layer.layer_id)));
    // auto& e1_layer = registry.get<CollidableComponent>(static_cast<entt::entity>(coll.ent_id_1));
    // auto& e1_layer_name = std::string(magic_enum::enum_name(static_cast<GameCollisionLayer>(e1_layer.layer_id)));
    // std::cout << e0_layer_name << " started colliding with: " << e1_layer_name << std::endl;
  }
  for (const auto& coll : p.collision_exit) {
    // auto& e0_layer = registry.get<CollidableComponent>(static_cast<entt::entity>(coll.ent_id_0));
    // auto& e0_layer_name = std::string(magic_enum::enum_name(static_cast<GameCollisionLayer>(e0_layer.layer_id)));
    // auto& e1_layer = registry.get<CollidableComponent>(static_cast<entt::entity>(coll.ent_id_1));
    // auto& e1_layer_name = std::string(magic_enum::enum_name(static_cast<GameCollisionLayer>(e1_layer.layer_id)));
    // std::cout << e0_layer_name << " stopped colliding with: " << e1_layer_name << std::endl;
  }

  ImGui::End();
}
