#include "system.hpp"

#include "lifecycle/components.hpp"
#include "modules/actors/helpers.hpp"
#include "modules/combat_attack_cooldown/components.hpp"
#include "modules/system_particles/components.hpp"

namespace game2d {

void
update_spawn_particles_on_death_system(entt::registry& r)
{
  const auto& view = r.view<RequestToSpawnParticles>(entt::exclude<WaitForInitComponent>);
  for (const auto& [e, req] : view.each()) {

    const auto particle_emitter = create_transform(r);
    set_size(r, particle_emitter, { 0, 0 }); // no size just script, but need position
    set_position(r, particle_emitter, req.position);

    // which particle to spawn?
    Particle desc;
    desc.sprite = "EMPTY";
    desc.colour = { 0.8f, 0.2f, 0.2f, 0.5f };
    desc.pos = req.position;
    desc.start_size = 16;
    desc.end_size = 4;
    desc.time_to_live_ms = 1000;

    ParticleEmitterComponent emitter;
    emitter.particle_to_emit = desc;
    emitter.expires = true;
    emitter.particles_to_spawn_before_emitter_expires = 10;
    emitter.random_velocity = true;
    emitter.spawn_all_particles_at_once = true;
    r.emplace<ParticleEmitterComponent>(particle_emitter, emitter);

    // emit: particles
    AttackCooldownComponent cooldown;
    cooldown.time_between_attack = 0.1f;
    cooldown.time_between_attack_left = cooldown.time_between_attack;
    r.emplace<AttackCooldownComponent>(particle_emitter, cooldown);
  }

  r.destroy(view.begin(), view.end()); // all requests processed
};

} // namespace game2d