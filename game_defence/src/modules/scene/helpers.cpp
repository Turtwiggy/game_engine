#include "helpers.hpp"

#include "actors.hpp"
#include "audio/components.hpp"
#include "entt/helpers.hpp"
#include "events/components.hpp"
#include "game_state.hpp"
#include "maths/grid.hpp"
#include "maths/maths.hpp"
#include "modules/actor_bow/components.hpp"
#include "modules/actor_cursor/components.hpp"
#include "modules/actor_dropoff_zone/components.hpp"
#include "modules/actor_pickup_zone/components.hpp"
#include "modules/camera/orthographic.hpp"
#include "modules/gameover/components.hpp"
#include "modules/grid/components.hpp"
#include "modules/items/helpers.hpp"
#include "modules/lifecycle/components.hpp"
#include "modules/renderer/components.hpp"
#include "physics/components.hpp"
#include "resources/colours.hpp"
#include "sprites/helpers.hpp"

namespace game2d {

void
move_to_scene_start(entt::registry& r, const Scene s)
{
  {
    // entt::registry new_r;
    // transfer<SINGLE_Animations>(r, new_r);
    // transfer<SINGLETON_Textures>(r, new_r);
    // transfer<SINGLETON_AudioComponent>(r, new_r);
    // transfer<SINGLETON_RendererInfo>(r, new_r);
    // transfer<SINGLETON_InputComponent>(r, new_r);
    // transfer<SINGLETON_FixedUpdateInputHistory>(r, new_r);
    // transfer<AudioSource>(r, new_r); // multiple of these
    // r = std::move(new_r);
  }

  const auto& transforms = r.view<TransformComponent>(entt::exclude<OrthographicCamera>);
  r.destroy(transforms.begin(), transforms.end());

  const auto& actors = r.view<EntityTypeComponent>(entt::exclude<OrthographicCamera>);
  r.destroy(actors.begin(), actors.end());

  destroy_and_create<SINGLETON_CurrentScene>(r);
  destroy_and_create<SINGLETON_PhysicsComponent>(r);
  destroy_and_create<SINGLETON_EntityBinComponent>(r);
  destroy_and_create<SINGLETON_GameStateComponent>(r);
  destroy_and_create<SINGLETON_ColoursComponent>(r);
  destroy_and_create<SINGLETON_GameOver>(r);
  destroy_and_create<SINGLETON_Wave>(r);

  if (s == Scene::game) {
    // create a cursor
    const auto cursor = create_gameplay(r, EntityType::cursor);

    // create a player
    auto e = create_gameplay(r, EntityType::actor_player);

    const int pixel_scale_up_size = 2;
    const auto& ri = get_first_component<SINGLETON_RendererInfo>(r);

    // create food/item dispencers
    for (int i = 0; i < 5; i++) {
      const auto e = create_gameplay(r, EntityType::actor_dispencer);
      auto& e_aabb = r.get<AABB>(e);
      e_aabb.center = glm::ivec2{ ((80 * i)), 0 };
      e_aabb.size = glm::ivec2{ 16 * pixel_scale_up_size, 16 * pixel_scale_up_size };
      auto& pz = r.get<PickupZoneComponent>(e);
      pz.spawn_item_with_id = i;
      // r.get<TransformComponent>(e).scale = glm::ivec3{ e_aabb.size.x, e_aabb.size.y, 1 };
      r.remove<TransformComponent>(e);

      // create delivery icon
      {
        const auto icon = create_gameplay(r, EntityType::empty);
        r.get<TransformComponent>(icon).position = { e_aabb.center.x, e_aabb.center.y, 0.0f };
        r.get<TransformComponent>(icon).scale = { e_aabb.size.x, e_aabb.size.y, 1.0f };
        const auto info = item_id_to_sprite(r, i);
        set_sprite(r, icon, info.sprite);
        auto& sc = r.get<SpriteComponent>(icon);
        sc.tex_unit = ri.tex_unit_custom;
        sc.total_sx = 32;
        sc.total_sy = 32;
      }

      // create delivery text
      {
        auto& anims = get_first_component<SINGLE_Animations>(r);
        const auto info = item_id_to_sprite(r, i);
        const auto anim = find_animation(anims, info.sprite_text);
        const auto& anim_pos = anim.animation_frames[0];
        const auto& w = anim_pos.w;

        const auto icon = create_gameplay(r, EntityType::empty);
        r.get<TransformComponent>(icon).position = { e_aabb.center.x, e_aabb.center.y - 40, 0.0f };
        r.get<TransformComponent>(icon).scale = { e_aabb.size.x * w, e_aabb.size.y, 1.0f };
        set_sprite(r, icon, info.sprite_text);
        auto& sc = r.get<SpriteComponent>(icon);
        sc.tex_unit = ri.tex_unit_custom;
        sc.total_sx = 32;
        sc.total_sy = 32;
        sc.tex_pos.w = anim_pos.w;
        sc.tex_pos.h = anim_pos.h;
      }
    }

    // create delivery points
    for (int i = 0; i < 1; i++) {
      const auto e = create_gameplay(r, EntityType::actor_customer_area);
      r.get<AABB>(e).center = glm::ivec2{ -150, -150 };
      r.get<AABB>(e).size = glm::ivec2{ 50, 50 };
      r.get<TransformComponent>(e).scale = glm::ivec3{ 50, 50, 50 };
    }
  }

  auto& scene = get_first_component<SINGLETON_CurrentScene>(r);
  scene.s = s; // done
}

} // namespace game2d