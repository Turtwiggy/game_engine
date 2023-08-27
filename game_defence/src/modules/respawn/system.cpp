#include "system.hpp"

#include "actors.hpp"
#include "entt/helpers.hpp"
#include "lifecycle/components.hpp"
#include "modules/actor_spawner/components.hpp"
#include "modules/combat_damage/components.hpp"
#include "modules/respawn/components.hpp"
#include "renderer/components.hpp"

namespace game2d {

void
update_respawn_system(entt::registry& r)
{
  auto& dead = get_first_component<SINGLETON_EntityBinComponent>(r);

  const auto& view = r.view<const HealthComponent, const EntityTypeComponent>();
  for (const auto& [e, hp, type] : view.each()) {
    if (hp.hp <= 0) {
      // is the unit able to respawn?

      if (auto* infinte_lives = r.try_get<InfiniteLivesComponent>(e)) {
        // yeah.. respawn it

        // find a spawner of the type that died and spawn it there
        for (const auto& [e_spawner, spawner] : r.view<SpawnerComponent>().each()) {
          if (spawner.type_to_spawn == type.type) {

            // spawn new entity at this spawner
            const auto& spawner_pos = r.get<TransformComponent>(e_spawner);
            CreateEntityRequest req;
            req.type = spawner.type_to_spawn;
            req.position = spawner_pos.position;
            r.emplace<CreateEntityRequest>(r.create(), req);

            break; // just choose the first spawner
          }
        }

        // design question: kill off the old player,
        // or just move its position and reset its state?
        // kill it off for now
        // dead.dead.emplace(e);
        std::cerr << "ur ded; what do with bow?" << std::endl;

      } else {
        dead.dead.emplace(e);
      }
    }
  }
}

} // namespace game2d