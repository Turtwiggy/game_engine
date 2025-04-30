#include "pch.hpp"

#include "enemy_grower_helpers.hpp"
#include "engine/physics/physics_helpers.hpp"
#include "modules/core/raws/raws_components.hpp"

namespace game2d {

void
update_circle_fixture_size(entt::registry& r, entt::entity body_e, entt::entity fix_e, float diameter_pixels)
{
  auto& fix_c = r.get<PhysicsFixtureComponent>(fix_e);
  auto* shape = static_cast<const b2CircleShape*>(fix_c.fixture->GetShape());
  const float old_radius_meters = shape->m_radius;
  const float old_radius_pixels = meters_to_pixels(old_radius_meters);

  float epsilon = 0.001f;
  const float difference = glm::abs(diameter_pixels - 2.0 * old_radius_pixels);
  if (difference < epsilon)
    return;
  SDL_Log("Creating new circle fixture...");

  // update shape...
  auto& body_c = r.get<PhysicsBodyComponent>(body_e);
  body_c.body->DestroyFixture(fix_c.fixture);
  fix_c.fixture = nullptr;

  // create with the new size...
  const auto& tag = r.get<TagComponent>(fix_e).tag;
  auto fixture_def = get_fixture_def_by_tag(r, body_e, tag);
  fixture_def.size_in_pixels.clear();
  fixture_def.size_in_pixels.push_back({ diameter_pixels, diameter_pixels });
  auto* new_fixture = create_fixture(body_c.body, fixture_def, { 0, 0 });

  // box2d: give link to entt
  new_fixture->GetUserData().pointer = (uint32)fix_e;

  fix_c.fixture = new_fixture;
};

} // namespace game2d