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
  for (const auto& [entity, transform] : r.view<TransformComponent>().each()) {
    r.destroy(entity);
  }

  destroy_and_create<SINGLETON_EntityBinComponent>(r);
  destroy_and_create<SINGLETON_GameStateComponent>(r);
  destroy_and_create<SINGLETON_Economy>(r);
  destroy_and_create<SINGLETON_ColoursComponent>(r);
  destroy_and_create<SINGLETON_GameOver>(r);

  const auto camera = create_gameplay(r, world, EntityType::camera);

  const auto hearth = create_gameplay(r, world, EntityType::actor_hearth);
  const auto& hearth_actor = r.get<ActorComponent>(hearth);
  hearth_actor.body->SetTransform({ 500.0f, 500.0f }, 0.0f);

  const auto& turret_0 = create_gameplay(r, world, EntityType::actor_turret);
  const auto& turret_0_actor = r.get<ActorComponent>(turret_0);
  turret_0_actor.body->SetTransform({ 600.0f, 600.0f }, 0.0f);

  const auto& turret_1 = create_gameplay(r, world, EntityType::actor_turret);
  const auto& turret_1_actor = r.get<ActorComponent>(turret_1);
  turret_1_actor.body->SetTransform({ 400.0f, 400.0f }, 0.0f);

  const auto& spawner = create_gameplay(r, world, EntityType::spawner);
  auto& spawner_actor = r.get<TransformComponent>(spawner);
  spawner_actor.position = { 0, 0, 0 };

  const auto player = create_gameplay(r, world, EntityType::actor_player);
  const auto& player_actor = r.get<ActorComponent>(player);
  player_actor.body->SetTransform({ 550.0f, 550.0f }, 0.0f);
}

} // namespace game2d