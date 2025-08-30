#include "pch.hpp"

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
  if (auto* pb = r.try_get<PhysicsBodyComponent>(e))
    return meters_to_pixels(b2Body_GetPosition(pb->bodyId));

  const auto& t = r.get<TransformComponent>(e);
  return { t.position.x, t.position.y };
};

glm::ivec2
get_grid_position(entt::registry& r, const entt::entity e)
{
  const auto& map = get_first_component<MapComponent>(r);
  const auto pos = get_position(r, e);
  return engine::grid::worldspace_to_gridspace(pos, map.tilesize);
};

void
set_position(entt::registry& r, const entt::entity e, const glm::vec2 pos_in_pixels)
{
  if (e == entt::null || !r.valid(e)) {
    SDL_Log("trying to set position of something invalid");
    return;
  }

  if (auto* pb = r.try_get<PhysicsBodyComponent>(e))
    b2Body_SetTransform(pb->bodyId, pixels_to_meters(pos_in_pixels), b2Rot_identity);

  auto& t_c = r.get<TransformComponent>(e);
  t_c.position = glm::vec3{ pos_in_pixels.x, pos_in_pixels.y, 0.0f };
};

void
set_position_grid(entt::registry& r, const entt::entity e, const glm::ivec2 gridpos)
{
  const auto& map_c = get_first_component<MapComponent>(r);
  const glm::ivec2 pos = engine::grid::gridspace_to_worldspace_center(gridpos, map_c.tilesize);
  set_position(r, e, pos);
};

void
set_rotation(entt::registry& r, const entt::entity e, const float angle)
{
  auto& t = r.get<TransformComponent>(e);
  t.rotation_radians.z = angle;

  if (auto* pb = r.try_get<PhysicsBodyComponent>(e))
    b2Body_SetTransform(pb->bodyId, b2Body_GetPosition(pb->bodyId), b2MakeRot(angle));
};

void
set_dir(entt::registry& r, const entt::entity e, const glm::vec2& dir)
{
  const auto angle = engine::dir_to_angle_radians(dir) - engine::PI;
  set_rotation(r, e, angle);
};

glm::vec2
get_fixture_size(entt::registry& r, const entt::entity fixture_e)
{
  const auto& fixture_c = r.get<PhysicsFixtureComponent>(fixture_e);
  const auto shapeId = fixture_c.shapeId;
  const auto aabb = b2Shape_GetAABB(shapeId);
  const float width = aabb.upperBound.x - aabb.lowerBound.x;
  const float height = aabb.upperBound.y - aabb.lowerBound.y;
  return meters_to_pixels({ width, height });
};

glm::vec2
get_size(entt::registry& r, const entt::entity e)
{
  if (auto* pb = r.try_get<PhysicsBodyComponent>(e)) {
    b2AABB aabb;
    aabb.lowerBound = b2Vec2(std::numeric_limits<float>::max(), std::numeric_limits<float>::max());
    aabb.upperBound = b2Vec2(std::numeric_limits<float>::lowest(), std::numeric_limits<float>::lowest());

    const auto body_aabb = b2Body_ComputeAABB(pb->bodyId);
    // const int count = b2Body_GetShapeCount(pb->bodyId);
    // std::vector<b2ShapeId> array;
    // array.resize(count);
    // b2Body_GetShapes(pb->bodyId, array.data(), count);
    // for (int i = 0; i < count; i++) {
    //   const b2ShapeId shape_id = array[i];
    //   const auto aabb = b2Shape_GetAABB(shape_id);
    // }

    const float width = body_aabb.upperBound.x - body_aabb.lowerBound.x;
    const float height = body_aabb.upperBound.y - body_aabb.lowerBound.y;

    return meters_to_pixels({ width, height });
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