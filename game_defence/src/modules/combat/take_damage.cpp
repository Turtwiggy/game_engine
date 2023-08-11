#include "take_damage.hpp"

#include "components.hpp"
#include "entt/helpers.hpp"
#include "flash_sprite.hpp"
#include "lifecycle/components.hpp"

namespace game2d {

void
update_take_damage_system(entt::registry& r)
{
  auto& dead = get_first_component<SINGLETON_EntityBinComponent>(r);

  const auto& view = r.view<DealDamageRequest>();
  for (auto [e_req, request] : view.each()) {

    // Does the attacker have an attack component?
    const auto* atk = r.try_get<AttackComponent>(request.from);

    // Does the defender have health?
    auto* hp = r.try_get<HealthComponent>(request.to);

    if (!atk)
      continue; // no attack damage given
    if (!hp)
      continue; // not able to take damage?

    // .. show flash
    r.emplace_or_replace<FlashOnDamageComponent>(request.to);

    // .. play audio
    // r.emplace<AudioRequestPlayEvent>(r.create(), "HIT");

    // .. popup some numbers as vfx
    // ..

    // .. take damage
    hp->hp -= glm::max(0, atk->damage);

    // check if dead
    if (hp->hp <= 0)
      dead.dead.emplace(request.to);

    r.destroy(e_req); // done request
  }
};

} // namespace game2d