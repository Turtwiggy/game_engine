#include "particle_system.hpp"

#include "components.hpp"
#include "engine/actors/actor_helpers.hpp"
#include "engine/lifecycle/components.hpp"
#include "engine/maths/maths.hpp"
#include "engine/renderer/transform.hpp"
#include "modules/actors/actor_rock/rock_helpers.hpp"
#include "modules/core/raws/raws_components.hpp"
#include "modules/core/renderer/helpers.hpp"
#include "modules/systems/system_cooldown/components.hpp"
#include "modules/systems/system_cooldown/helpers.hpp"
#include "modules/systems/system_move_to_target_via_lerp/components.hpp"

namespace game2d {

int
get_idx(const float time, const float duration, const int size)
{
  if (duration == 0.0f)
    return 0;
  const float percent = time / duration;             // a value between 0 and 1
  const int index = (int)std::floor(percent * size); // a value between 0 and size
  return std::clamp(index, 0, size);                 // check between 0 and size
};

void
update_particle_system(entt::registry& r, const float dt)
{
#if defined(_DEBUG)
  ZoneScoped;
#endif
  const auto spawn_particle_helper = [&r](const ParticleEmitterComponent& emitter, const entt::entity e) {
    // per-instance? seems bad
    Particle pd = emitter.particle_to_emit;

    // spawn at e
    pd.position = get_position(r, e);

    if (auto* target_c = r.try_get<DynamicTargetComponent>(e)) {
      if (!r.valid(target_c->target)) {
        r.remove<DynamicTargetComponent>(e);
        return;
      }
      pd.position = get_position(r, target_c->target);
    }

    static engine::RandomState rnd;

    // adjust the position inside a circle
    float distance = 0;
    auto adj_pos = pd.position;
    if (pd.random_radius_bound_upper > 0.0f) {
      distance = engine::rand_det_s(rnd.rng, pd.random_radius_bound_lower, pd.random_radius_bound_upper);
      const float angle = engine::rand_det_s(rnd.rng, 0.0f, 2.0f * engine::PI);
      const auto dir = engine::angle_radians_to_direction(angle);
      const auto ray = engine::Ray{
        .origin = { pd.position.x, pd.position.y, 0.0f },
        .dir = { dir.x, dir.y, 0.0f },
      };
      const auto p = engine::ray_at(ray, distance);
      adj_pos.x = p.x;
      adj_pos.y = p.y;
    }

    // adjust colour based on distance from non-adj pos
    if (pd.make_darker_based_on_distance_from_center) {
      const auto col = pd.start_colour;
      const auto min_distance = pd.random_radius_bound_lower;
      const auto max_distance = pd.random_radius_bound_upper;
      const auto percent = 1.0f - (glm::abs(distance) / (max_distance - min_distance));
      pd.start_colour = lerp_colour({ 0.0f, 0.0f, 0.0f, 1.0f }, pd.start_colour, percent);
    }

    // set velocity
    const int rnd_x = engine::rand_det_s(rnd.rng, -pd.random_velocity_bound, pd.random_velocity_bound);
    const int rnd_y = engine::rand_det_s(rnd.rng, -pd.random_velocity_bound, pd.random_velocity_bound);
    pd.velocity = glm::ivec2{ rnd_x, rnd_y };

    // get your position vs your adj position, and set the vel as the dir
    if (pd.velocity_in_dir && adj_pos != pd.position) {
      const glm::vec2 dir = engine::normalize_safe(adj_pos - pd.position);
      pd.velocity = 50.0f * dir;

      if (!pd.velocity_away)
        pd.velocity *= -1;
    }

    pd.position = adj_pos;
    const entt::entity particle_e = spawn_particle(r, "default_particle", pd);
  };

  // spawn the particles
  const auto& view = r.view<ParticleEmitterComponent, CooldownComponent>(entt::exclude<WaitForInitComponent>);
  for (const auto& [particle_emitter_e, emitter, cooldown] : view.each()) {

    if (cooldown.time <= 0.0f) {

      if (emitter.spawn_all_particles_at_once) {
        for (int i = 0; i < emitter.particles_to_spawn_before_emitter_expires; i++) {
          spawn_particle_helper(emitter, particle_emitter_e);
          // emitter.particles_to_spawn_before_emitter_expires--;
        }

        if (emitter.expires)
          r.destroy(particle_emitter_e);

      } else {
        spawn_particle_helper(emitter, particle_emitter_e);
        emitter.particles_to_spawn_before_emitter_expires--;

        // limit number of particles spawned
        if (emitter.expires && emitter.particles_to_spawn_before_emitter_expires < 0) {
          r.destroy(particle_emitter_e); // emitter expired!
          continue;
        }
      }

      reset_cooldown(cooldown);
    }
  }

  // Scale particles
  const auto& particle_view = r.view<TransformComponent, ScaleOverTimeComponent, const EntityTimedLifecycle>();
  for (const auto& [e, transform, scale, life] : particle_view.each()) {

    // todo make parametric not linear

    float t = scale.timer / scale.seconds_until_complete;
    if (t >= 1.0f)
      t = 1.0f;

    if (t >= 1.0f)
      continue; // done

    const auto i0 = get_idx(scale.timer, scale.seconds_until_complete, (int)(scale.size_curve.size() - 1));
    const auto a = scale.size_curve[i0];
    const auto b = scale.size_curve[i0 + 1];

    const float amount_x = engine::lerp(a.x, b.x, t);
    const float amount_y = engine::lerp(a.y, b.y, t);
    transform.scale = { amount_x, amount_y, 1 };

    scale.timer += dt;
    if (scale.timer >= scale.seconds_until_complete)
      scale.timer = scale.seconds_until_complete; // done?
  }
};

} // namespace game2d
