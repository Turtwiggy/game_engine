#include "pch.hpp"

#include "actor_snake_projectiles_system.hpp"

#include "engine/actors/actor_helpers.hpp"
#include "engine/maths/maths.hpp"
#include "engine/physics/physics_components.hpp"
#include "modules/actors/actor_snake/snake_components.hpp"
#include "modules/combat/combat_projectiles/projectile_components.hpp"
#include "modules/combat/combat_projectiles/projectile_helpers.hpp"
#include "modules/core/colour/components.hpp"
#include "modules/systems/system_cooldown/components.hpp"
#include "modules/systems/system_cooldown/helpers.hpp"
#include "modules/ui/ui_colours/ui_colours_helpers.hpp"

namespace game2d {

const auto col = hex_to_srgb("#FF0000");

void
update_actor_snake_projectiles_system(entt::registry& r)
{
  auto view = r.view<SnakeSegment, CooldownComponent, PhysicsBodyComponent>();
  for (const auto& [e, snake_segment_c, cooldown_c, body_c] : view.each()) {
    if (cooldown_c.time > 0.0f)
      continue;
    reset_cooldown(cooldown_c);

    const auto pos = get_position(r, e);
    const auto dir = engine::angle_radians_to_direction(body_c.body->GetAngle());
    const auto pos_perp = glm::vec2{ -dir.y, dir.x };
    const auto neg_perp = -1.0f * pos_perp;

    const auto bullet_speed = 1.20f;
    const auto bullet_size = glm::vec2{ 12, 12 };

    BulletDef bullet_def(e); // note: not a weapon parent, but an enemy
    bullet_def.key = "bullet_snake";
    bullet_def.size = bullet_size;
    bullet_def.team = AvailableTeams::enemy;
    bullet_def.damage = 2; // TODO: make enemy bullet correct damage
    bullet_def.pierce = 1;
    bullet_def.speed = bullet_speed;
    bullet_def.lifecycle = 10 * 1000;
    bullet_def.damage_type = WEAPON_DAMAGE::KINETIC;

    const auto bullet_e_0 = spawn_projectile(r, bullet_def, pos);
    const auto bullet_e_1 = spawn_projectile(r, bullet_def, pos);
    set_colour(r, bullet_e_0, col);
    set_colour(r, bullet_e_1, col);
    r.get<DefaultColour>(bullet_e_0).colour = col;
    r.get<DefaultColour>(bullet_e_1).colour = col;

    // set velocity
    auto& bul_body_c_0 = r.get<PhysicsBodyComponent>(bullet_e_0);
    auto& bul_body_c_1 = r.get<PhysicsBodyComponent>(bullet_e_1);
    bul_body_c_0.body->SetLinearVelocity({ bullet_speed * pos_perp.x, bullet_speed * pos_perp.y });
    bul_body_c_1.body->SetLinearVelocity({ bullet_speed * neg_perp.x, bullet_speed * neg_perp.y });

    //
  }
}

} // namespace game2d