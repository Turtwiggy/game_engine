#include "actor_helpers.hpp"

#include "engine/colour/colour.hpp"
#include "engine/entt/helpers.hpp"
#include "engine/map/components.hpp"
#include "engine/maths/grid.hpp"
#include "engine/maths/maths.hpp"
#include "engine/physics/physics_components.hpp"
#include "engine/physics/physics_helpers.hpp"
#include "engine/renderer/transform.hpp"
#include "engine/sprites/components.hpp"

namespace game2d {

glm::vec2
get_position(entt::registry& r, const entt::entity e)
{
  if (auto* pb = r.try_get<PhysicsBodyComponent>(e)) {
    const auto& pos = pb->body->GetPosition();
    return { pos.x, pos.y };
  }
  const auto& t = r.get<TransformComponent>(e);
  return { t.position.x, t.position.y };
};

glm::ivec2
get_grid_position(entt::registry& r, const entt::entity e)
{
  const auto& map = get_first_component<MapComponent>(r);
  const auto pos = get_position(r, e);
  return engine::grid::worldspace_to_grid_space(pos, map.tilesize);
}

void
set_position(entt::registry& r, const entt::entity e, const glm::vec2& pos)
{
  if (auto* pb = r.try_get<PhysicsBodyComponent>(e)) {
    pb->body->SetTransform(b2Vec2{ pos.x, pos.y }, 0);

    // Hack: some unit positions are set via lerp.
    // SetAwake(true) will try to make sure the contacts are updated,
    // so the GetCollisonEnter() event will still fire.
    // pb->body->SetAwake(true);
  }
  r.get<TransformComponent>(e).position = { pos.x, pos.y, 0.0f };
}

void
set_position_grid(entt::registry& r, const entt::entity e, const glm::ivec2 gridpos)
{
  const auto& map_c = get_first_component<MapComponent>(r);
  const glm::ivec2 pos = engine::grid::grid_space_to_world_space_center(gridpos, map_c.tilesize);
  set_position(r, e, pos);
}

void
set_dir(entt::registry& r, const entt::entity e, const glm::vec2& dir)
{
  const auto angle = engine::dir_to_angle_radians(dir) - engine::PI;

  auto& t = r.get<TransformComponent>(e);
  t.rotation_radians.z = angle;

  if (auto* pb = r.try_get<PhysicsBodyComponent>(e))
    pb->body->SetTransform(pb->body->GetPosition(), angle);
}

glm::vec2
get_size(entt::registry& r, const entt::entity e)
{
  if (auto* pb = r.try_get<PhysicsBodyComponent>(e)) {
    b2AABB aabb;
    aabb.lowerBound = b2Vec2(std::numeric_limits<float>::max(), std::numeric_limits<float>::max());
    aabb.upperBound = b2Vec2(std::numeric_limits<float>::lowest(), std::numeric_limits<float>::lowest());

    for (const b2Fixture* fixture = pb->body->GetFixtureList(); fixture; fixture = fixture->GetNext()) {
      const b2Shape* shape = fixture->GetShape();

      // Get the number of vertices
      int32 childCount = shape->GetChildCount();
      for (int32 i = 0; i < childCount; ++i) {
        b2AABB shapeAABB;
        shape->ComputeAABB(&shapeAABB, pb->body->GetTransform(), i);
        aabb.Combine(shapeAABB);
      }
    }

    const float width = aabb.upperBound.x - aabb.lowerBound.x;
    const float height = aabb.upperBound.y - aabb.lowerBound.y;
    return glm::vec2{ width, height };
  }
  const auto& transform = r.get<TransformComponent>(e);
  return { transform.scale.x, transform.scale.y };
};

void
set_size(entt::registry& r, const entt::entity e, const glm::vec2& size)
{
  // WARNING: does not set physics size currently
  // This is because one entity could have multiple fixtures.
  // How to correctly resize all fixtures?
  // Easy case: if 1 fixture, just resize it with size
  // Hard case: if 1+ fixtures...

  // if (auto* pb = r.try_get<PhysicsBodyComponent>(e)) {
  //   SDL_Log("Warning; destroying and creating new fixtures for physics object");
  //   // Destroy all existing fixtures
  //   for (b2Fixture* fixture = pb->body->GetFixtureList(); fixture != nullptr;) {
  //     b2Fixture* nextFixture = fixture->GetNext();
  //     pb->body->DestroyFixture(fixture);
  //     fixture = nextFixture;
  //   }
  //   auto& physd = r.get<PhysicsDescription>(e);
  //   physd.size = size;
  //   create_box_fixture(r, e, pb->body);
  // }

  auto& transform = r.get<TransformComponent>(e);
  transform.scale.x = size.x;
  transform.scale.y = size.y;
};

void
set_colour(entt::registry& r, const entt::entity e, const engine::SRGBColour& col)
{
  auto& sc = r.get<SpriteComponent>(e);
  sc.colour = engine::SRGBToLinear(col);
};

} // namespace game2d