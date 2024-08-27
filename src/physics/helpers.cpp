#include "helpers.hpp"

#include "actors/base.hpp"
#include "entt/helpers.hpp"
#include "modules/combat_damage/components.hpp"
#include "physics/components.hpp"
#include "renderer/transform.hpp"

#include <box2d/b2_circle_shape.h>
#include <fmt/core.h>

namespace game2d {

void
emplace_or_replace_physics_world(entt::registry& r)
{
  // store one physics world...
  static b2World* world = new b2World(b2Vec2(0.0f, 0.0f));

  // cleanup physics world...
  {
    static bool needs_deleting = false;
    if (needs_deleting) {
      b2Joint* joint = world->GetJointList();
      while (joint) {
        b2Joint* j = joint;
        joint = joint->GetNext();
        world->DestroyJoint(j);
      }
      b2Body* body = world->GetBodyList();
      while (body) {
        b2Body* b = body;
        body = body->GetNext();
        world->DestroyBody(b);
      }
    }
    needs_deleting = true;
  }

  destroy_first_and_create<SINGLE_Physics>(r, SINGLE_Physics{ world });
};

void
create_physics_actor(entt::registry& r, const entt::entity e, const PhysicsDescription& desc)
{
  auto& physics_c = get_first_component<SINGLE_Physics>(r);

  // Bodies are built using the following steps:
  // Define a body with position, damping, etc.
  // Use the world object to create the body.
  // Define fixtures with a shape, friction, density, etc.
  // Create fixtures on the body.
  b2Body* body = nullptr;

  // create a body
  {
    b2BodyDef body_def;
    body_def.position.Set(desc.position.x, desc.position.y);
    body_def.angle = 0.0f;
    body_def.fixedRotation = true;
    body_def.bullet = desc.is_bullet;
    body_def.type = desc.type;
    body_def.linearVelocity = b2Vec2_zero;
    body_def.angularDamping = desc.angular_damping;
    body = physics_c.world->CreateBody(&body_def);

    // set user data as the entity id
    body->GetUserData().pointer = (uintptr_t)e;
  }

  // create a fixture
  //
  b2PolygonShape box;
  box.SetAsBox(desc.size.x / 2.0f, desc.size.y / 2.0f);

  // b2CircleShape circle;

  b2FixtureDef fixture_def;
  fixture_def.friction = 0.0f;
  fixture_def.density = desc.density;
  fixture_def.shape = &box;
  fixture_def.isSensor = desc.is_sensor;

  body->CreateFixture(&fixture_def);

  //

  PhysicsBodyComponent body_c;
  body_c.body = body;
  r.emplace<PhysicsBodyComponent>(e, body_c);

  // While we're creating it, update the transform
  auto& transform_c = r.get<TransformComponent>(e);
  transform_c.scale.x = desc.size.x;
  transform_c.scale.y = desc.size.y;
};

/*
void
set_collision_filters(entt::registry& r, entt::entity e)
{
  // I am a ....
  constexpr uint16 FRIENDLY_UNIT_CATEGORY = 0x0001;
  constexpr uint16 FRIENDLY_BULLET_CATEGORY = 0x0002;
  constexpr uint16 ENEMY_UNIT_CATEGORY = 0x0004;
  constexpr uint16 ENEMY_BULLET_CATEGORY = 0x0008;
  constexpr uint16 WALL_CATEGORY = 0x0016;

  // I collide with ... (0xFFFF is everything)
  constexpr uint16_t FRIENDLY_UNIT_MASK = WALL_CATEGORY | ENEMY_BULLET_CATEGORY;
  constexpr uint16_t FRIENDLY_BULLET_MASK = WALL_CATEGORY | ENEMY_UNIT_CATEGORY;
  constexpr uint16_t ENEMY_UNIT_MASK = WALL_CATEGORY | FRIENDLY_BULLET_CATEGORY;
  constexpr uint16_t ENEMY_BULLET_MASK = WALL_CATEGORY | FRIENDLY_UNIT_CATEGORY;
  constexpr uint16_t WALL_MASK = 0xFFFF;

  const auto& physics_body = r.get<PhysicsBodyComponent>(e);
  const auto type = r.get<EntityTypeComponent>(e).type;
  const auto* team_c = r.try_get<TeamComponent>(e);

  return; // currently not doing this

  for (b2Fixture* fixture = physics_body.body->GetFixtureList(); fixture; fixture = fixture->GetNext()) {

    b2Filter filter;

    if (type == EntityType::solid_wall) {
      filter.categoryBits = WALL_CATEGORY;
      filter.maskBits = WALL_MASK;
      fixture->SetFilterData(filter);
    }

    if (type == EntityType::bullet_default || type == EntityType::bullet_bouncy) {
      if (team_c->team == AvailableTeams::player) {
        filter.categoryBits = FRIENDLY_BULLET_CATEGORY;
        filter.maskBits = FRIENDLY_BULLET_MASK;
        fixture->SetFilterData(filter);
      } else if (team_c->team == AvailableTeams::enemy) {
        filter.categoryBits = ENEMY_BULLET_CATEGORY;
        filter.maskBits = ENEMY_BULLET_MASK;
        fixture->SetFilterData(filter);
      } else {
        fmt::println("warning; no team set on bullet. not setting physics masks");
      }
    }

    if (type == EntityType::actor_dungeon) {
      if (team_c->team == AvailableTeams::player) {
        filter.categoryBits = FRIENDLY_UNIT_CATEGORY;
        filter.maskBits = FRIENDLY_UNIT_MASK;
        fixture->SetFilterData(filter);
      } else if (team_c->team == AvailableTeams::enemy) {
        filter.categoryBits = ENEMY_UNIT_CATEGORY;
        filter.maskBits = ENEMY_UNIT_MASK;
        fixture->SetFilterData(filter);
      } else
        fmt::println("warning; no team set on actor_dungeon. not setting physics masks");
    }
  }
}
*/

} // namespace game2d