#include "system.hpp"

#include "actors.hpp"
#include "entt/helpers.hpp"
#include "modules/actor_spawner/components.hpp"
#include "modules/combat_damage/components.hpp"
#include "modules/lifecycle/components.hpp"
#include "modules/renderer/components.hpp"
#include "modules/respawn/components.hpp"

#include <algorithm>

namespace game2d {

void
update_respawn_system(entt::registry& r)
{
  auto& dead = get_first_component<SINGLETON_EntityBinComponent>(r);

  const auto& view = r.view<HealthComponent>(entt::exclude<WaitForInitComponent>);
  for (const auto& [e, hp] : view.each()) {
    if (hp.hp <= 0) {
      // is the unit able to respawn?

      if (const auto* infinte_lives = r.try_get<InfiniteLivesComponent>(e)) {
        // just set its hp back to full
        hp.hp = hp.max_hp;
        std::cout << "unit died; set back to full hp" << std::endl;

        // if it was in the dead list, un-dead it
        // const auto in_dead_list =
        //   std::find_if(dead.dead.begin(), dead.dead.end(), [&e](const auto& dead_e) { return dead_e == e; });
        // if (in_dead_list != dead.dead.end())
        //   dead.dead.erase(in_dead_list);

        // yeah.. respawn it
        // find a spawner of the type that died and spawn it there
        // for (const auto& [e_spawner, spawner] : r.view<SpawnerComponent>(entt::exclude<WaitForInitComponent>).each()) {
        //   if (spawner.type_to_spawn == type.type) {

        //     // spawn new entity at this spawner
        //     const auto& spawner_transform = r.get<TransformComponent>(e_spawner);

        //     // the entity
        //     const auto req = create_gameplay(r, spawner.type_to_spawn);
        //     r.get<TransformComponent>(req).position = spawner_transform.position;

        //     break; // just choose the first spawner
        //   }
        // }

        // design question: kill off the old player,
        // or just move its position and reset its state?
        // kill it off for now
        // dead.dead.emplace(e);
        std::cout << "ur ded; what do with bow?" << std::endl;

      } else {
        dead.dead.emplace(e);
      }
    }
  }
}

} // namespace game2d