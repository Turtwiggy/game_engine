// your header
#include "systems/process_physics.hpp"

// components
#include "modules/physics/components.hpp"

// helpers
#include "helpers/physics_layers.hpp"

// c++ lib headers
#include <algorithm>
#include <iostream>

void
game2d::update_process_physics_system(entt::registry& registry, engine::Application& app, float dt)
{
  SINGLETON_PhysicsComponent& p = registry.ctx<SINGLETON_PhysicsComponent>();

  for (const auto& coll : p.collision_enter) {
    CollidableComponent& e0_layer = registry.get<CollidableComponent>(static_cast<entt::entity>(coll.ent_id_0));
    auto e0_layer_id = e0_layer.layer_id;
    CollidableComponent& e1_layer = registry.get<CollidableComponent>(static_cast<entt::entity>(coll.ent_id_1));
    auto e1_layer_id = e1_layer.layer_id;

    // Now run through resolutions?
    // HmmmmmMMMMMMMMmmmmmmmmMMMMMM this seems bad
    //
    auto min_id = static_cast<GameCollisionLayer>(std::min(e0_layer_id, e1_layer_id));
    auto max_id = static_cast<GameCollisionLayer>(std::max(e0_layer_id, e1_layer_id));

    if (min_id == GameCollisionLayer::ACTOR_PLAYER && max_id == GameCollisionLayer::SOLID_WALL) {
      std::cout << "PLAYER-WALL collision..." << std::endl;
    }

    // else if (min_id == GameCollisionLayer::SOLID_WALL && max_id == GameCollisionLayer::SOLID_WALL) {
    //   std::cout << "WALL-WALL collision..." << std::endl;
    // } else
    // } else if (min_id == GameCollisionLayer::ACTOR_GOAL && max_id == GameCollisionLayer::SOLID_WALL) {
    //   std::cout << "GOAL-WALL collision..." << std::endl;
    // else if (min_id == GameCollisionLayer::SOLID_WALL && max_id == GameCollisionLayer::ACTOR_BALL) {
    //   std::cout << "WALL-BALL collision..." << std::endl; }

    if (min_id == GameCollisionLayer::ACTOR_PLAYER && max_id == GameCollisionLayer::ACTOR_PLAYER) {
      std::cout << "PLAYER-PLAYER collision..." << std::endl;
    } else if (min_id == GameCollisionLayer::ACTOR_PLAYER && max_id == GameCollisionLayer::ACTOR_GOAL) {
      std::cout << "PLAYER-GOAL collision..." << std::endl;
    } else if (min_id == GameCollisionLayer::ACTOR_PLAYER && max_id == GameCollisionLayer::ACTOR_BALL) {
      std::cout << "PLAYER-BALL collision..." << std::endl;
    } else if (min_id == GameCollisionLayer::ACTOR_GOAL && max_id == GameCollisionLayer::ACTOR_GOAL) {
      std::cout << "GOAL-GOAL collision..." << std::endl;
    } else if (min_id == GameCollisionLayer::ACTOR_GOAL && max_id == GameCollisionLayer::ACTOR_BALL) {
      std::cout << "GOAL-BALL collision..." << std::endl;
      std::cout << "You win a cookie!" << std::endl;
    } else if (min_id == GameCollisionLayer::ACTOR_BALL && max_id == GameCollisionLayer::ACTOR_BALL) {
      std::cout << "BALL-BALL collision..." << std::endl;
    }
  }
}