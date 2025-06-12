#include "pch.hpp"

#include "enemy_grower_helpers.hpp"
#include "engine/physics/physics_helpers.hpp"
#include "modules/core/raws/raws_components.hpp"

namespace game2d {

void
update_circle_fixture_size(entt::registry& r, entt::entity body_e, entt::entity fix_e, float diameter_pixels)
{
  auto& fix_c = r.get<PhysicsFixtureComponent>(fix_e);

  const auto shapeId = fix_c.shapeId;
  const b2Circle circle = b2Shape_GetCircle(shapeId);
  const float old_radius_meters = circle.radius;
  const float old_radius_pixels = meters_to_pixels(old_radius_meters);

  float epsilon = 0.001f;
  const float difference = glm::abs(diameter_pixels - 2.0f * old_radius_pixels);
  if (difference < epsilon)
    return;
  SDL_Log("Creating new circle fixture...");
  b2DestroyShape(shapeId, true);
  fix_c.shapeId = b2_nullShapeId;

  // create with the new size...
  const auto& tag = r.get<TagComponent>(fix_e).tag;
  auto fixture_def = get_fixture_def_by_tag(r, body_e, tag);
  fixture_def.size_in_pixels.clear();
  fixture_def.size_in_pixels.push_back({ diameter_pixels, diameter_pixels });

  const auto new_shapeId = create_fixture(fix_c.bodyId, fixture_def, { 0, 0 });
  b2Shape_SetUserData(new_shapeId, (void*)static_cast<uintptr_t>(entt::to_integral(fix_e)));
  fix_c.shapeId = new_shapeId;
};

} // namespace game2d