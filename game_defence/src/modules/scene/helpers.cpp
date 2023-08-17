#include "helpers.hpp"

#include "actors.hpp"
#include "audio/components.hpp"
#include "entt/helpers.hpp"
#include "events/components.hpp"
#include "game_state.hpp"
#include "lifecycle/components.hpp"
#include "modules/combat/helpers.hpp"
#include "modules/gameover/components.hpp"
#include "modules/physics/components.hpp"
#include "modules/ui_economy/components.hpp"
#include "renderer/components.hpp"
#include "resources/colours.hpp"
#include "resources/textures.hpp"
#include "ui_profiler/components.hpp"

namespace game2d {

void
move_to_scene_start(entt::registry& r, const Scene& s)
{
  {
    // entt::registry new_r;
    // transfer<SINGLETON_Animations>(r, new_r);
    // transfer<SINGLETON_Textures>(r, new_r);
    // transfer<SINGLETON_AudioComponent>(r, new_r);
    // transfer<SINGLETON_Profiler>(r, new_r);
    // transfer<SINGLETON_RendererInfo>(r, new_r);
    // transfer<SINGLETON_InputComponent>(r, new_r);
    // transfer<SINGLETON_FixedUpdateInputHistory>(r, new_r);
    // transfer<AudioSource>(r, new_r); // multiple of these
    // r = std::move(new_r);
  }

  for (const auto& [entity, comp] : r.view<TransformComponent>().each())
    r.destroy(entity);
  for (const auto& [entity, comp] : r.view<EntityTypeComponent>().each())
    r.destroy(entity);
  for (const auto& [entity, comp] : r.view<CreateEntityRequest>().each())
    r.destroy(entity);

  destroy_and_create<SINGLETON_CurrentScene>(r);
  destroy_and_create<SINGLETON_PhysicsComponent>(r);
  destroy_and_create<SINGLETON_EntityBinComponent>(r);
  destroy_and_create<SINGLETON_GameStateComponent>(r);
  destroy_and_create<SINGLETON_Economy>(r);
  destroy_and_create<SINGLETON_UiEconomy>(r);
  destroy_and_create<SINGLETON_ColoursComponent>(r);
  destroy_and_create<SINGLETON_GameOver>(r);
  const auto camera = create_gameplay(r, EntityType::camera);
  auto& camera_transform = r.get<TransformComponent>(camera);
  camera_transform.position = { 50, 50, 0 };
  camera_transform.scale.x = 0;
  camera_transform.scale.y = 0;

  if (s == Scene::game) {

    const auto hearth = create_gameplay(r, EntityType::actor_hearth);
    auto& hearth_transform = r.get<TransformComponent>(hearth);
    const glm::ivec2 spawn_pos = { 500, 500 };
    hearth_transform.position = { spawn_pos.x, spawn_pos.y, 0 };
    camera_transform.position = { spawn_pos.x, spawn_pos.y, 0 };

    const auto player = create_gameplay(r, EntityType::actor_player);
    auto& player_transform = r.get<TransformComponent>(player);
    player_transform.position = { 550, 550, 0 };

    const auto spawner = create_gameplay(r, EntityType::spawner);
    auto& spawner_transform = r.get<TransformComponent>(spawner);
    spawner_transform.position = { 700, 700, 0 };
  }

  auto& scene = get_first_component<SINGLETON_CurrentScene>(r);
  scene.s = s; // done
}

} // namespace game2d