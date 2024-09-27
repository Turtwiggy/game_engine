#include "system.hpp"

#include "actors/actor_helpers.hpp"
#include "engine/lifecycle/components.hpp"
#include "modules/raws/raws_components.hpp"
#include "modules/system_particles/components.hpp"

namespace game2d {

void
update_spawn_particles_on_death_system(entt::registry& r)
{
  const auto& view = r.view<RequestToSpawnParticles>(entt::exclude<WaitForInitComponent>);
  for (const auto& [e, req] : view.each()) {

    const auto emitter_parent_e = create_transform(r, "emitter-parent");
    set_size(r, emitter_parent_e, { 0, 0 }); // no size just script, but need position
    set_position(r, emitter_parent_e, req.position);

    r.emplace<EntityTimedLifecycle>(emitter_parent_e, 1 * 1000);
    spawn_particle_emitter(r, "default_explode", req.position, emitter_parent_e);
  }

  r.destroy(view.begin(), view.end()); // all requests processed
};

} // namespace game2d