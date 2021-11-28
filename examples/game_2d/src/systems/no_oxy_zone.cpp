// your header
#include "systems/no_oxy_zone.hpp"

// components
#include "components/health.hpp"
#include "modules/physics/components.hpp"
#include "modules/physics/helpers.hpp"
#include "modules/renderer/components.hpp"

// helpers
#include "helpers/physics_layers.hpp"

// other lib headers
#include <imgui.h>

void
game2d::update_no_oxy_zone_system(entt::registry& registry, engine::Application& app, float dt)
{
  SINGLETON_PhysicsComponent& p = registry.ctx<SINGLETON_PhysicsComponent>();

  // ImGui::Begin("Player Health");

  for (const auto& coll : p.collision_stay) {
    CollidableComponent& e0_layer = registry.get<CollidableComponent>(static_cast<entt::entity>(coll.ent_id_0));
    auto e0_layer_id = e0_layer.layer_id;
    CollidableComponent& e1_layer = registry.get<CollidableComponent>(static_cast<entt::entity>(coll.ent_id_1));
    auto e1_layer_id = e1_layer.layer_id;
    auto min_id = static_cast<GameCollisionLayer>(std::min(e0_layer_id, e1_layer_id));
    auto max_id = static_cast<GameCollisionLayer>(std::max(e0_layer_id, e1_layer_id));

    if (min_id == GameCollisionLayer::ACTOR_PLAYER && max_id == GameCollisionLayer::ACTOR_NO_OXY_ZONE) {
      // std::cout << "PLAYER-OXY_ZONE collision..." << std::endl;
      if (registry.all_of<HealthComponent>(static_cast<entt::entity>(coll.ent_id_0))) {
        HealthComponent& h = registry.get<HealthComponent>(static_cast<entt::entity>(coll.ent_id_0));
        h.hp -= 1.0f * dt;
        // ImGui::Text("HP %f", h.hp);
      }
      if (registry.all_of<HealthComponent>(static_cast<entt::entity>(coll.ent_id_1))) {
        HealthComponent& h = registry.get<HealthComponent>(static_cast<entt::entity>(coll.ent_id_1));
        h.hp -= 1.0f * dt;
        // ImGui::Text("HP %f", h.hp);
      }
    }
  }

  // ImGui::End();

  // for(const auto& coll: p.collision_stay){
  // //
  // }

  // for (const auto& coll : p.collision_exit) {
  // continue;
  // }
};