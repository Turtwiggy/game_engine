// your header
#include "systems/process_physics.hpp"

// components
#include "modules/physics/components.hpp"

// helpers
#include "helpers/physics_layers.hpp"

// other lib headers
#include "thirdparty/magic_enum.hpp"
#include <imgui.h>

void
game2d::update_process_physics_system(entt::registry& registry, engine::Application& app, float dt)
{
  SINGLETON_PhysicsComponent& p = registry.ctx<SINGLETON_PhysicsComponent>();

  ImGui::Begin("Game collision ui...", NULL, ImGuiWindowFlags_NoFocusOnAppearing);
  ImGui::Text("Collisions %i", p.filtered_collisions.size());

  // These are game logic collisions?
  for (const auto& collision : p.filtered_collisions) {
    auto& e0_layer = registry.get<CollidableComponent>(static_cast<entt::entity>(collision.second.ent_id_0));
    auto& e0_layer_name = std::string(magic_enum::enum_name(static_cast<GameCollisionLayer>(e0_layer.layer_id)));
    auto& e1_layer = registry.get<CollidableComponent>(static_cast<entt::entity>(collision.second.ent_id_1));
    auto& e1_layer_name = std::string(magic_enum::enum_name(static_cast<GameCollisionLayer>(e1_layer.layer_id)));

    ImGui::Text("Collision between: %i %i, layers: %s %s",
                collision.second.ent_id_0,
                collision.second.ent_id_1,
                e0_layer_name.c_str(),
                e1_layer_name.c_str());
  }

  // There's basically 3 states needed to capture:
  // OnCollisionEnter
  // OnCollisionStay
  // OnCollisionExit

  ImGui::End();
};