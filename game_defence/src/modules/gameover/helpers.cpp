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
  const auto& hearth_actor = r.get<PhysicsActorComponent>(hearth);
  const glm::vec2 spawn_pos = { 500.0f, 500.0f };

  // TODO: this
  // hearth_actor.body->SetTransform({ spawn_pos.x, spawn_pos.y }, 0.0f);
  // camera_transform.position = { spawn_pos.x, spawn_pos.y, 0.0f };

  const auto player = create_gameplay(r, EntityType::actor_player);
  const auto& player_actor = r.get<PhysicsActorComponent>(player);
  // TODO: this
  // player_actor.body->SetTransform({ 550.0f, 550.0f }, 0.0f);
}

} // namespace game2d