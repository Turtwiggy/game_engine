#include "turret.hpp"

#include "engine/maths/maths.hpp"
#include "game/create_entities.hpp"
#include "modules/lifecycle/components.hpp"
#include "modules/physics/components.hpp"
#include "modules/renderer/components.hpp"

#include "imgui.h"

void
game2d::update_turret_system(entt::registry& r)
{
  const auto& p = r.ctx().at<SINGLETON_PhysicsComponent>();
  auto& rnd = r.ctx().at<SINGLETON_ResourceComponent>();
  auto& eb = r.ctx().at<SINGLETON_EntityBinComponent>();
  const float dt = ImGui::GetIO().DeltaTime; // think this is wrong

  const auto& view = r.view<TurretComponent, TransformComponent>();
  view.each([&r, &rnd, &dt](auto entity, auto& turret, auto& transform) {
    // for each turret...

    turret.time_since_last_shot += dt;

    if (turret.time_since_last_shot > turret.time_between_shots) {
      turret.time_since_last_shot -= turret.time_between_shots;

      // .. get a new random direction
      const float angle = engine::rand_det_s(rnd.rnd.rng, 0.0f, engine::PI * 2);
      glm::vec2 dir = engine::angle_radians_to_direction(angle);

      // .. fire a projectile
      entt::entity bullet = create_bullet(r);
      auto& bullet_velocity = r.get<VelocityComponent>(bullet);
      bullet_velocity.x = dir.x * turret.bullet_speed;
      bullet_velocity.y = dir.y * turret.bullet_speed;
      auto& bullet_transform = r.get<TransformComponent>(bullet);
      bullet_transform.position = transform.position;
      bullet_transform.rotation.z = angle;

      // .. set the turret's angle
      transform.rotation.z = angle;
    }
  });

  //
  // Resolve any asteroid-turret collisions
  //
  {
    entt::entity e0_turret = entt::null;
    entt::entity e1_asteroid = entt::null;

    for (const auto& coll : p.collision_enter) {

      const auto e0_id = static_cast<entt::entity>(coll.ent_id_0);
      const auto& e0_layer = r.get<PhysicsActorComponent>(e0_id);
      const auto e0_layer_id = e0_layer.layer_id;

      const auto e1_id = static_cast<entt::entity>(coll.ent_id_1);
      const auto& e1_layer = r.get<PhysicsActorComponent>(e1_id);
      const auto e1_layer_id = e1_layer.layer_id;

      // Collisions are bi-directional, but only one collision exists
      if (e0_layer_id == GameCollisionLayer::ACTOR_PLAYER_PET && e1_layer_id == GameCollisionLayer::ACTOR_ASTEROID) {
        e0_turret = e0_id;
        e1_asteroid = e1_id;

        // resolve...
        eb.dead.emplace(e0_turret);
        eb.dead.emplace(e1_asteroid);
      }
      if (e0_layer_id == GameCollisionLayer::ACTOR_ASTEROID && e1_layer_id == GameCollisionLayer::ACTOR_PLAYER_PET) {
        e0_turret = e1_id;
        e1_asteroid = e0_id;

        // resolve...
        eb.dead.emplace(e0_turret);
        eb.dead.emplace(e1_asteroid);
      }
    }
  }
};