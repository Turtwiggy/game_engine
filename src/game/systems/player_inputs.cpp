#include "player_inputs.hpp"

#include "game/entities/actors.hpp"
#include "modules/events/components.hpp"
#include "modules/events/helpers/keyboard.hpp"
#include "modules/events/helpers/mouse.hpp"
#include "modules/lifecycle/components.hpp"
#include "modules/physics/components.hpp"
#include "modules/renderer/components.hpp"

#include "engine/maths/maths.hpp"

#include <imgui.h>

// maybe the player queue should have a
// list of all inputs pressed by a player

void
game2d::update_player_inputs_system(entt::registry& r)
{
  const auto& p = r.ctx().at<SINGLETON_PhysicsComponent>();
  auto& gameover = r.ctx().at<SINGLETON_GameOverComponent>();
  auto& eb = r.ctx().at<SINGLETON_EntityBinComponent>();
  auto& input = r.ctx().at<SINGLETON_InputComponent>();

  //
  // Capture player(s) inputs
  //

  const auto& view = r.view<PlayerComponent>();
  view.each([&input, &r](auto entity, auto& player) {
    // just capture inputs and process them in fixedupdate()

    if (get_key_down(input, player.W))
      input.unprocessed_update_inputs.push_back(
        { INPUT_TYPE::KEYBOARD, entity, false, static_cast<uint32_t>(player.W) });
    if (get_key_down(input, player.A))
      input.unprocessed_update_inputs.push_back(
        { INPUT_TYPE::KEYBOARD, entity, false, static_cast<uint32_t>(player.A) });
    if (get_key_down(input, player.S))
      input.unprocessed_update_inputs.push_back(
        { INPUT_TYPE::KEYBOARD, entity, false, static_cast<uint32_t>(player.S) });
    if (get_key_down(input, player.D))
      input.unprocessed_update_inputs.push_back(
        { INPUT_TYPE::KEYBOARD, entity, false, static_cast<uint32_t>(player.D) });

    if (get_key_up(input, player.W))
      input.unprocessed_update_inputs.push_back(
        { INPUT_TYPE::KEYBOARD, entity, true, static_cast<uint32_t>(player.W) });
    if (get_key_up(input, player.A))
      input.unprocessed_update_inputs.push_back(
        { INPUT_TYPE::KEYBOARD, entity, true, static_cast<uint32_t>(player.A) });
    if (get_key_up(input, player.S))
      input.unprocessed_update_inputs.push_back(
        { INPUT_TYPE::KEYBOARD, entity, true, static_cast<uint32_t>(player.S) });
    if (get_key_up(input, player.D))
      input.unprocessed_update_inputs.push_back(
        { INPUT_TYPE::KEYBOARD, entity, true, static_cast<uint32_t>(player.D) });

    if (get_mouse_lmb_press())
      input.unprocessed_update_inputs.push_back({ INPUT_TYPE::MOUSE, entity, false, SDL_BUTTON_LEFT });
    if (get_mouse_lmb_release())
      input.unprocessed_update_inputs.push_back({ INPUT_TYPE::MOUSE, entity, true, SDL_BUTTON_LEFT });

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