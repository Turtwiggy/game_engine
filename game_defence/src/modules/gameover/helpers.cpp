#include "helpers.hpp"

#include "actors.hpp"
#include "entt/helpers.hpp"
#include "game_state.hpp"
#include "lifecycle/components.hpp"
#include "modules/combat/helpers.hpp"
#include "modules/gameover/components.hpp"
#include "modules/physics/components.hpp"
#include "modules/ui_economy/components.hpp"
#include "renderer/components.hpp"
#include "resources/colours.hpp"

namespace game2d {

void
restart_game(entt::registry& r)
{
  for (const auto& [entity, transform] : r.view<TransformComponent>().each())
    r.destroy(entity);

  destroy_and_create<SINGLETON_PhysicsComponent>(r);
  destroy_and_create<SINGLETON_EntityBinComponent>(r);
  destroy_and_create<SINGLETON_GameStateComponent>(r);
  destroy_and_create<SINGLETON_Economy>(r);
  destroy_and_create<SINGLETON_UiEconomy>(r);
  destroy_and_create<SINGLETON_ColoursComponent>(r);
  destroy_and_create<SINGLETON_GameOver>(r);

  const auto camera = create_gameplay(r, EntityType::camera);
  auto& camera_transform = r.get<TransformComponent>(camera);

  // load a map?

  const auto hearth = create_gameplay(r, EntityType::actor_hearth);
  auto& hearth_transform = r.get<TransformComponent>(hearth);
  const glm::ivec2 spawn_pos = { 500, 500 };
  hearth_transform.position = { spawn_pos.x, spawn_pos.y, 0 };
  camera_transform.position = { spawn_pos.x, spawn_pos.y, 0 };

  const auto player = create_gameplay(r, EntityType::actor_player);
  auto& player_transform = r.get<TransformComponent>(player);
  player_transform.position = { 550, 550, 0 };
}

} // namespace game2d