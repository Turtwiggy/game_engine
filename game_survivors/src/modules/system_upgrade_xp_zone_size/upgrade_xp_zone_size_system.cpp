#include "upgrade_xp_zone_size_system.hpp"

#include "modules/core_raws/raws_components.hpp"
#include "upgrade_xp_zone_size_components.hpp"

#include "engine/entt/helpers.hpp"
#include "engine/physics/physics_components.hpp"
#include "engine/physics/physics_helpers.hpp"
#include "modules/actor_player/components.hpp"
#include "modules/system_upgrade/upgrade_components.hpp"

#include <box2d/b2_fixture.h>
#include <glm/glm.hpp>
#include <magic_enum.hpp>

namespace game2d {

void
update_upgrade_xp_zone_size_system(entt::registry& r)
{
  auto& physics_c = get_first_component<SINGLE_Physics>(r);

  auto view = r.view<const PlayerComponent, const ActorXpZoneSizeComponent, const StatModifierComponent>();
  for (const auto& [e, player_c, xp_zone_c, stats_c] : view.each()) {

    const auto key = std::string(magic_enum::enum_name(UpgradeableStat::ACTOR_XP_ZONE_SIZE));
    const auto val = xp_zone_c.radius_meters;
    const auto val_mod = stats_c.apply_modifiers(val, key);

    auto fixture_e = get_fixture_by_tag(r, e, "fixture_xp_zone");
    auto& fixture_c = r.get<PhysicsFixtureComponent>(fixture_e);

    const b2Shape* shape = fixture_c.fixture->GetShape();
    if (shape->GetType() != b2Shape::e_circle) {
      // oops - xp zone is no longer a circle?
      throw std::runtime_error("XP zone is no longer a circle?");
      continue;
    }
    const auto* circle = static_cast<const b2CircleShape*>(shape);
    const float radius = circle->m_radius;

    const float epsilon = 0.001f;
    const float difference = glm::abs(val_mod - radius);
    if (difference > epsilon) {
      // size different. remake fixture.
      SDL_Log("XP zone radius changed... creating new fixture");

      // destroy
      if (fixture_c.fixture) {
        fixture_c.body->DestroyFixture(fixture_c.fixture);
        fixture_c.fixture = nullptr;
      }

      // create with the new size
      auto fixture_def = get_fixture_def_by_tag(r, e, "fixture_xp_zone");
      fixture_def.size.clear(); // removes default value
      fixture_def.size.push_back({ val_mod, 0 });
      auto* new_fixture = create_fixture(fixture_c.body, fixture_def, { 0, 0 });

      // box2d: give link to entt
      new_fixture->GetUserData().pointer = (uint32)fixture_e;

      fixture_c.fixture = new_fixture;
    }
  }
}

} // namespace game2d