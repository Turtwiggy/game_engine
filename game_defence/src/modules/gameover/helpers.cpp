#include "helpers.hpp"

#include "actors.hpp"
#include "entt/helpers.hpp"
#include "game_state.hpp"
#include "lifecycle/components.hpp"
#include "modules/gameover/components.hpp"
#include "modules/physics_box2d/components.hpp"
#include "modules/ui_economy/components.hpp"
#include "renderer/components.hpp"
#include "resources/colours.hpp"

namespace game2d {

void
restart_game(entt::registry& r, b2World& world)
{
  for (const auto& [entity, actor] : r.view<ActorComponent>().each()) {
    world.DestroyBody(actor.body);
    r.destroy(entity);
  }
  for (const auto& [entity, transform] : r.view<TransformComponent>().each())
    r.destroy(entity);

  destroy_and_create<SINGLETON_EntityBinComponent>(r);
  destroy_and_create<SINGLETON_GameStateComponent>(r);
  destroy_and_create<SINGLETON_Economy>(r);
  destroy_and_create<SINGLETON_ColoursComponent>(r);
  destroy_and_create<SINGLETON_GameOver>(r);

  const auto camera = create_gameplay(r, world, EntityType::camera);

  const auto hearth = create_gameplay(r, world, EntityType::actor_hearth);
  const auto& hearth_actor = r.get<ActorComponent>(hearth);
  hearth_actor.body->SetTransform({ 500.0f, 500.0f }, 0.0f);

  // const auto& spawner0 = create_gameplay(r, world, EntityType::spawner);
  // auto& spawner0_actor = r.get<TransformComponent>(spawner0);
  // spawner0_actor.position = { 0, 0, 0 };

  // const auto& spawner1 = create_gameplay(r, world, EntityType::spawner);
  // auto& spawner1_actor = r.get<TransformComponent>(spawner1);
  // spawner1_actor.position = { 1000, 1000, 0 };

  const auto player = create_gameplay(r, world, EntityType::actor_player);
  const auto& player_actor = r.get<ActorComponent>(player);
  player_actor.body->SetTransform({ 550.0f, 550.0f }, 0.0f);
}

} // namespace game2d