#include "system.hpp"

#include "audio/components.hpp"
#include "components.hpp"
#include "entt/helpers.hpp"
#include "modules/combat_flash_on_damage/components.hpp"
#include "modules/combat_flash_on_damage/helpers.hpp"
#include "modules/lifecycle/components.hpp"
#include "modules/screenshake/components.hpp"
#include "physics/components.hpp"
#include "renderer/helpers.hpp"
#include "sprites/helpers.hpp"

#include "maths/maths.hpp"

namespace game2d {

void
update_take_damage_system(entt::registry& r)
{
  const auto& view = r.view<DealDamageRequest>(entt::exclude<WaitForInitComponent>);
  for (auto [e_req, request] : view.each()) {

    // Does the attacker have an attack component?
    const auto* atk = r.try_get<AttackComponent>(request.from);

    // Does the defender have health?
    auto* hp = r.try_get<HealthComponent>(request.to);

    if (!atk)
      continue; // no attack damage given
    if (!hp)
      continue; // not able to take damage?

    // Does the defender have the ability to be knocked back?
    if (auto* v = r.try_get<VelocityComponent>(request.to)) {
      const auto& get_position = [&r](const entt::entity& e) -> glm::ivec2 {
        auto* aabb = r.try_get<AABB>(e);
        if (aabb)
          return aabb->center;
        else {
          const auto& t = r.get<TransformComponent>(e);
          return { t.position.x, t.position.y };
        }
      };
      const glm::vec2 atk_pos = get_position(request.from);
      const glm::vec2 def_pos = get_position(request.to);
      const auto dir = glm::normalize(atk_pos - def_pos);

      const float knockback_amount = 5.0f;
      v->remainder_x += -dir.x * knockback_amount;
      v->remainder_y += -dir.y * knockback_amount;
    }

    // .. show as flashing
    r.emplace_or_replace<RequestFlashComponent>(request.to);

    // .. play audio
    r.emplace<AudioRequestPlayEvent>(r.create(), "TAKE_DAMAGE_01");

    // .. screenshake
    r.emplace<RequestScreenshakeComponent>(r.create());

    // .. popup some numbers as vfx
    const int text_seperation = 6;
    const auto sprites = convert_int_to_sprites(atk->damage);
    const auto def_transform = r.get<TransformComponent>(request.to);
    static engine::RandomState rnd;
    const float rnd_x = engine::rand_det_s(rnd.rng, -50, 50);
    const float rnd_y = engine::rand_det_s(rnd.rng, -50, 50);
    for (int i = 0; i < sprites.size(); i++) {
      const auto req = create_gameplay(r, EntityType::particle);
      VelocityComponent vel;
      vel.x = rnd_x;
      vel.y = rnd_y;
      r.emplace_or_replace<VelocityComponent>(req, vel);
      glm::vec3 offset_pos = def_transform.position;
      offset_pos.x += (i)*text_seperation;
      set_position(r, req, offset_pos);
      set_sprite(r, req, sprites[i]);
    }

    // .. take damage
    hp->hp -= glm::max(0, atk->damage);
  }

  // done all damage requests
  r.destroy(view.begin(), view.end());

  // NEXT: check if anything is dead
  // NOTE: this is done in the respawn system
};

} // namespace game2d