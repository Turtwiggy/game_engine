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
#include "modules/animation/components.hpp"
#include "modules/camera/orthographic.hpp"
#include "modules/gameover/components.hpp"
#include "modules/grid/components.hpp"
#include "modules/items/helpers.hpp"
#include "modules/lifecycle/components.hpp"
#include "modules/procedural/poisson.hpp"
#include "modules/procedural/voronoi.hpp"
#include "modules/renderer/components.hpp"
#include "modules/renderer/helpers.hpp"
#include "modules/screenshake/components.hpp"
#include "physics/components.hpp"
#include "resources/colours.hpp"
#include "sprites/helpers.hpp"

#include <string>

namespace game2d {
using namespace std::literals;

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
  destroy_and_create<SINGLE_ScreenshakeComponent>(r);

  // create a cursor
  const auto cursor = create_gameplay(r, EntityType::cursor);

  if (s == Scene::game) {
    const auto& anims = get_first_component<SINGLE_Animations>(r);
    const auto& ri = get_first_component<SINGLETON_RendererInfo>(r);
    const auto& colours = get_first_component<SINGLETON_ColoursComponent>(r);
    const int pixel_scale_up_size = 2;
    const auto default_size = glm::ivec2{ 16 * pixel_scale_up_size, 16 * pixel_scale_up_size };
    const auto tex_unit = search_for_texture_by_path(ri, "bargame")->unit;

    // create a player
    {
      const auto e = create_gameplay(r, EntityType::actor_player);
      auto& e_aabb = r.get<AABB>(e);
      e_aabb.size = default_size * 1;
      e_aabb.center = { -88, -30 };

      const auto icon_xy = set_sprite_custom(r, e, "player_0", tex_unit);
      r.get<TransformComponent>(e).scale = { e_aabb.size.x, e_aabb.size.y, 1.0f };
    }

    // create food/item dispencers
    for (int i = 0; i < 5; i++) {
      const auto e = create_gameplay(r, EntityType::actor_dispencer);
      auto& e_aabb = r.get<AABB>(e);
      e_aabb.center = glm::ivec2{ ((80 * i)), 0 };
      e_aabb.size = default_size;
      auto& pz = r.get<PickupZoneComponent>(e);
      pz.spawn_item_with_id = i;
      r.get<TransformComponent>(e).scale = glm::ivec3{ e_aabb.size.x, e_aabb.size.y, 1 };
      r.remove<SpriteComponent>(e);
      // r.get<SpriteComponent>(e).colour = engine::SRGBToLinear(engine::SRGBColour(1.0f, 1.0f, 1.0f, 0.5f));
      // r.get<SpriteComponent>(e).colour.a = 0.0f;
      // r.remove<TransformComponent>(e);

      // create delivery icon
      {
        const auto icon = create_gameplay(r, EntityType::empty);
        r.get<TransformComponent>(icon).position = { e_aabb.center.x, e_aabb.center.y, 0.0f };
        r.get<TransformComponent>(icon).scale = { e_aabb.size.x, e_aabb.size.y, 1.0f };

        const auto info = item_id_to_sprite(r, i);
        set_sprite_custom(r, icon, info.sprite, tex_unit);

        WiggleUpAndDown wiggle;
        wiggle.base_position = e_aabb.center;
        wiggle.offset = i; // offset the wiggles!
        r.emplace<WiggleUpAndDown>(icon, wiggle);
      }

      // create delivery text
      {
        const auto info = item_id_to_sprite(r, i);
        const auto icon = create_gameplay(r, EntityType::empty);
        const auto icon_xy = set_sprite_custom(r, icon, info.sprite_text, tex_unit);

        r.get<TransformComponent>(icon).position = { e_aabb.center.x, e_aabb.center.y - 30, 0.0f };
        r.get<TransformComponent>(icon).scale = { e_aabb.size.x * icon_xy.x, e_aabb.size.y, 1.0f };
      }

      // create delivery table
      {
        const auto icon = create_gameplay(r, EntityType::empty);
        const auto icon_xy = set_sprite_custom(r, icon, "icon_table_green"s, tex_unit);

        auto& t = r.get<TransformComponent>(icon);
        t.position = { e_aabb.center.x, e_aabb.center.y + 30, 0.0f };
        t.scale = { (e_aabb.size.x * icon_xy.x), e_aabb.size.y, 1.0f };
        // auto& aabb = r.get<AABB>(icon);
        // aabb.center = { t.position.x, t.position.y };
        // aabb.size = { t.scale.x - 50, t.scale.y };
        r.emplace<PhysicsSolidComponent>(icon);
      }
    }

    // create delivery points
    {
      for (int i = 0; i < 1; i++) {
        const auto e = create_gameplay(r, EntityType::actor_customer_area);
        auto& e_aabb = r.get<AABB>(e);
        {
          e_aabb.center = glm::ivec2{ -150, -150 };
          e_aabb.size = default_size;

          const auto icon_xy = set_sprite_custom(r, e, "campfire_empty"s, tex_unit);
          auto& icon_transform = r.get<TransformComponent>(e);
          icon_transform.position = { e_aabb.center.x, e_aabb.center.y, 0.0f };
          icon_transform.scale = { e_aabb.size.x * icon_xy.x, e_aabb.size.y * icon_xy.y, 1.0f };
        }

        // show sign above delivery point
        {
          const auto icon = create_gameplay(r, EntityType::empty);
          const auto icon_xy = set_sprite_custom(r, icon, "icon_sign_inactive"s, tex_unit);

          auto& icon_transform = r.get<TransformComponent>(icon);
          icon_transform.position = { e_aabb.center.x, e_aabb.center.y - 100.0f, 0.0f };
          icon_transform.scale = { e_aabb.size.x * icon_xy.x, e_aabb.size.y * icon_xy.y, 1.0f };
          r.get<DropoffZoneComponent>(e).sign = icon;
        }
      }
    }

    // use poisson for grass
    const int width = 1920;
    const int height = 1080;
    const auto poisson = generate_poisson(width, height, 150, 0);
    const glm::ivec2 offset = { -width / 2, -height / 2 };
    std::cout << "generated " << poisson.size() << " poisson points" << std::endl;

    for (const auto& p : poisson) {
      const auto icon = create_gameplay(r, EntityType::empty);
      const auto icon_xy = set_sprite_custom(r, icon, "icon_grass"s, tex_unit);

      r.get<TransformComponent>(icon).position = { offset.x + p.x, offset.y + p.y, 0.0f };
      r.get<TransformComponent>(icon).scale = { default_size.x, default_size.y, 1.0f };
      r.get<TagComponent>(icon).tag = "grass"s;
    }
  }

  auto& scene = get_first_component<SINGLETON_CurrentScene>(r);
  scene.s = s; // done
}

} // namespace game2d