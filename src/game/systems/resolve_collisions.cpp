#include "resolve_collisions.hpp"

void
game2d::update_resolve_collisions_system(entt::registry& r){
  //
  // Resolve player-asteroid collisions
  //
  // {
  //   entt::entity e0_player = entt::null;
  //   entt::entity e1_asteroid = entt::null;

  //   for (const auto& coll : p.collision_enter) {

  //     const auto e0_id = static_cast<entt::entity>(coll.ent_id_0);
  //     const auto& e0_layer = r.get<PhysicsActorComponent>(e0_id);
  //     const auto e0_layer_id = e0_layer.layer_id;

  //     const auto e1_id = static_cast<entt::entity>(coll.ent_id_1);
  //     const auto& e1_layer = r.get<PhysicsActorComponent>(e1_id);
  //     const auto e1_layer_id = e1_layer.layer_id;

  //     // Collisions are bi-directional, but only one collision exists
  //     if (e0_layer_id == GameCollisionLayer::ACTOR_PLAYER && e1_layer_id == GameCollisionLayer::ACTOR_ENEMY) {
  //       e0_player = e0_id;
  //       e1_asteroid = e1_id;

  //       // resolve...
  //       eb.dead.emplace(e0_player); // destroy the bullet
  //       gameover.over = true;       // end the game
  //     }
  //     if (e0_layer_id == GameCollisionLayer::ACTOR_ASTEROID && e1_layer_id == GameCollisionLayer::ACTOR_PLAYER) {
  //       e0_player = e1_id;
  //       e1_asteroid = e0_id;

  //       // resolve...
  //       eb.dead.emplace(e0_player); // destroy the bullet
  //       gameover.over = true;       // end the game
  //     }
  //   }
  // }

  //
  // Resolve bullet-asteroid collisions
  //
  // {
  //   entt::entity e0_bullet = entt::null;
  //   entt::entity e1_asteroid = entt::null;

  //   for (const auto& coll : p.collision_enter) {

  //     const auto e0_id = static_cast<entt::entity>(coll.ent_id_0);
  //     const auto& e0_layer = r.get<PhysicsActorComponent>(e0_id);
  //     const auto e0_layer_id = e0_layer.layer_id;

  //     const auto e1_id = static_cast<entt::entity>(coll.ent_id_1);
  //     const auto& e1_layer = r.get<PhysicsActorComponent>(e1_id);
  //     const auto e1_layer_id = e1_layer.layer_id;

  //     // Collisions are bi-directional, but only one collision exists
  //     if (e0_layer_id == GameCollisionLayer::ACTOR_BULLET && e1_layer_id == GameCollisionLayer::ACTOR_ASTEROID) {
  //       e0_bullet = e0_id;
  //       e1_asteroid = e1_id;

  //       // resolve...
  //       eb.dead.emplace(e0_bullet);   // destroy the bullet
  //       eb.dead.emplace(e1_asteroid); // destroy the asteroid
  //     }
  //     if (e0_layer_id == GameCollisionLayer::ACTOR_ASTEROID && e1_layer_id == GameCollisionLayer::ACTOR_BULLET) {
  //       e0_bullet = e1_id;
  //       e1_asteroid = e0_id;

  //       // resolve...
  //       eb.dead.emplace(e0_bullet);   // destroy the bullet
  //       eb.dead.emplace(e1_asteroid); // destroy the asteroid
  //     }
  //   }
  // }
};