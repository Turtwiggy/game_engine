#include "helpers.hpp"

#include "entt/helpers.hpp"
#include "renderer/transform.hpp"

namespace game2d {

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
    body = physics_c.world->CreateBody(&body_def);

    // set user data as the entity id
    body->GetUserData().pointer = (uintptr_t)e;
  }

  // create a fixture
  {
    b2PolygonShape box;
    box.SetAsBox(desc.size.x / 2.0f, desc.size.y / 2.0f);

    b2FixtureDef fixture_def;
    fixture_def.friction = 0.0f;
    fixture_def.density = desc.density;
    fixture_def.shape = &box;
    body->CreateFixture(&fixture_def);

    // Set collision filtering
    // const uint16 PLAYER_CATEGORY = 0x0001;
    // const uint16 BULLET_CATEGORY = 0x0002;
    // const uint16 WALL_CATEGORY = 0x0004;
    // fixtureDef.filter.categoryBits = PLAYER_CATEGORY;
    // fixtureDef.filter.maskBits = WALL_CATEGORY | BULLET_CATEGORY; // Collide with walls and bullets
  }

  PhysicsBodyComponent body_c;
  body_c.body = body;
  r.emplace<PhysicsBodyComponent>(e, body_c);

  // While we're creating it, update the transform
  auto& transform_c = r.get<TransformComponent>(e);
  transform_c.scale.x = desc.size.x;
  transform_c.scale.y = desc.size.y;
}

} // namespace game2d