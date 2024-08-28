#include "helpers.hpp"

#include "colour/colour.hpp"
#include "entt/helpers.hpp"
#include "maths/grid.hpp"
#include "modules/grid/components.hpp"
#include "modules/ui_colours/helpers.hpp"
#include "physics/components.hpp"
#include "renderer/transform.hpp"
#include "sprites/components.hpp"

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

[[nodiscard]] glm::ivec2
get_grid_position(entt::registry& r, const entt::entity e)
{
  const auto& map = get_first_component<MapComponent>(r);
  const auto pos = get_position(r, e);
  return engine::grid::worldspace_to_grid_space(pos, map.tilesize);
}

void
set_position(entt::registry& r, const entt::entity e, const glm::vec2& pos)
{
  if (auto* pb = r.try_get<PhysicsBodyComponent>(e))
    pb->body->SetTransform(b2Vec2{ pos.x, pos.y }, 0);
  r.get<TransformComponent>(e).position = { pos.x, pos.y, 0.0f };
}

void
set_position_grid(entt::registry& r, const entt::entity e, const glm::ivec2 gridpos)
{
  const auto& map_c = get_first_component<MapComponent>(r);
  const glm::ivec2 pos = engine::grid::grid_space_to_world_space_center(gridpos, map_c.tilesize);
  set_position(r, e, pos);
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
  if (auto* pb = r.try_get<PhysicsBodyComponent>(e)) {

    // Destroy all existing fixtures
    for (b2Fixture* fixture = pb->body->GetFixtureList(); fixture != nullptr;) {
      b2Fixture* nextFixture = fixture->GetNext();
      pb->body->DestroyFixture(fixture);
      fixture = nextFixture;
    }

    // Create a new fixture with the updated size
    b2PolygonShape newBoxShape;
    newBoxShape.SetAsBox(size.x / 2.0f, size.y / 2.0f);

    // add fixture
    b2FixtureDef newFixtureDef;
    newFixtureDef.shape = &newBoxShape;
    newFixtureDef.density = 1.0f;
    newFixtureDef.friction = 0.3f;
    pb->body->CreateFixture(&newFixtureDef);
  }

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

void
set_colour_by_tag(entt::registry& r, const entt::entity e, const std::string& tag)
{
  const engine::SRGBColour col = get_srgb_colour_by_tag(r, tag);
  set_colour(r, e, col);
};

} // namespace game2d