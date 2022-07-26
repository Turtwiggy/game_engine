#include "player.hpp"

#include "game/components/game.hpp"
#include "game/create_entities.hpp"
#include "modules/events/components.hpp"
#include "modules/events/helpers/keyboard.hpp"
#include "modules/events/helpers/mouse.hpp"
#include "modules/lifecycle/components.hpp"
#include "modules/physics/components.hpp"
#include "modules/renderer/components.hpp"

#include "engine/maths/maths.hpp"

void
game2d::update_player_system(entt::registry& r)
{
  const auto& input = r.ctx().at<SINGLETON_InputComponent>();
  const auto& p = r.ctx().at<SINGLETON_PhysicsComponent>();
  auto& gameover = r.ctx().at<SINGLETON_GameOverComponent>();
  auto& eb = r.ctx().at<SINGLETON_EntityBinComponent>();

  //
  // Move player(s)
  //

  const auto& view = r.view<const PlayerComponent, VelocityComponent, TransformComponent>();
  view.each([&input, &r](const auto& player, auto& vel, auto& transform) {
    if (get_key_held(input, SDL_SCANCODE_W))
      vel.y = -1 * player.speed;
    if (get_key_held(input, SDL_SCANCODE_S))
      vel.y = 1 * player.speed;
    if (get_key_held(input, SDL_SCANCODE_A))
      vel.x = -1 * player.speed;
    if (get_key_held(input, SDL_SCANCODE_D))
      vel.x = 1 * player.speed;

    if (get_key_up(input, SDL_SCANCODE_A) || get_key_up(input, SDL_SCANCODE_D))
      vel.x = 0.0f;
    if (get_key_up(input, SDL_SCANCODE_W) || get_key_up(input, SDL_SCANCODE_S))
      vel.y = 0.0f;

    // Shoot()
    // if (get_mouse_lmb_press()) {
    //   entt::entity bullet = create_bullet(r);
    //   const int BULLET_SPEED = 500;
    //   const auto& mouse_pos = input.mouse_position_in_worldspace;
    //   glm::vec2 dir = { mouse_pos.x - transform.position.x, mouse_pos.y - transform.position.y };
    //   if (dir.x != 0.0f && dir.y != 0.0f)
    //     dir = glm::normalize(dir);
    //   auto& bullet_velocity = r.get<VelocityComponent>(bullet);
    //   bullet_velocity.x = dir.x * BULLET_SPEED;
    //   bullet_velocity.y = dir.y * BULLET_SPEED;
    //   auto& bullet_transform = r.get<TransformComponent>(bullet);
    //   bullet_transform.position = transform.position;
    //   float angle = engine::dir_to_angle_radians(dir);
    //   bullet_transform.rotation.z = angle - engine::HALF_PI;
    // }

    // .. rotate to velocity
    // .. IMPROVEMENT
    // Could improve this by not immediately setting rotation.z
    // and by setting a target angle that is lerped towards over multiple frames
    // const float EPSILON = 0.005f;
    // if (glm::abs(vel.x) > EPSILON || glm::abs(vel.y) > EPSILON)
    //   transform.rotation.z = engine::dir_to_angle_radians({ vel.x, vel.y }) - engine::HALF_PI;
  });

  //
  // Resolve player-asteroid collisions
  //
  {
    entt::entity e0_player = entt::null;
    entt::entity e1_asteroid = entt::null;

    for (const auto& coll : p.collision_enter) {

      const auto e0_id = static_cast<entt::entity>(coll.ent_id_0);
      const auto& e0_layer = r.get<PhysicsActorComponent>(e0_id);
      const auto e0_layer_id = e0_layer.layer_id;

      const auto e1_id = static_cast<entt::entity>(coll.ent_id_1);
      const auto& e1_layer = r.get<PhysicsActorComponent>(e1_id);
      const auto e1_layer_id = e1_layer.layer_id;

      // Collisions are bi-directional, but only one collision exists
      if (e0_layer_id == GameCollisionLayer::ACTOR_PLAYER && e1_layer_id == GameCollisionLayer::ACTOR_ASTEROID) {
        e0_player = e0_id;
        e1_asteroid = e1_id;

        // resolve...
        eb.dead.emplace(e0_player); // destroy the bullet
        gameover.over = true;       // end the game
      }
      if (e0_layer_id == GameCollisionLayer::ACTOR_ASTEROID && e1_layer_id == GameCollisionLayer::ACTOR_PLAYER) {
        e0_player = e1_id;
        e1_asteroid = e0_id;

        // resolve...
        eb.dead.emplace(e0_player); // destroy the bullet
        gameover.over = true;       // end the game
      }
    }
  }

  //
  // Resolve bullet-asteroid collisions
  //
  {
    entt::entity e0_bullet = entt::null;
    entt::entity e1_asteroid = entt::null;

    for (const auto& coll : p.collision_enter) {

      const auto e0_id = static_cast<entt::entity>(coll.ent_id_0);
      const auto& e0_layer = r.get<PhysicsActorComponent>(e0_id);
      const auto e0_layer_id = e0_layer.layer_id;

      const auto e1_id = static_cast<entt::entity>(coll.ent_id_1);
      const auto& e1_layer = r.get<PhysicsActorComponent>(e1_id);
      const auto e1_layer_id = e1_layer.layer_id;

      // Collisions are bi-directional, but only one collision exists
      if (e0_layer_id == GameCollisionLayer::ACTOR_BULLET && e1_layer_id == GameCollisionLayer::ACTOR_ASTEROID) {
        e0_bullet = e0_id;
        e1_asteroid = e1_id;

        // resolve...
        eb.dead.emplace(e0_bullet);   // destroy the bullet
        eb.dead.emplace(e1_asteroid); // destroy the asteroid
      }
      if (e0_layer_id == GameCollisionLayer::ACTOR_ASTEROID && e1_layer_id == GameCollisionLayer::ACTOR_BULLET) {
        e0_bullet = e1_id;
        e1_asteroid = e0_id;

        // resolve...
        eb.dead.emplace(e0_bullet);   // destroy the bullet
        eb.dead.emplace(e1_asteroid); // destroy the asteroid
      }
    }
  }
};