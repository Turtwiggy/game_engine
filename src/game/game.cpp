// your header
#include "game.hpp"

// systems&components&helpers
#include "modules/audio/system.hpp"
#include "modules/cursor/system.hpp"
#include "modules/events/components.hpp"
#include "modules/events/helpers/keyboard.hpp"
#include "modules/events/system.hpp"
#include "modules/lifecycle/components.hpp"
#include "modules/networking/system.hpp"
#include "modules/physics/components.hpp"
#include "modules/renderer/components.hpp"
#include "modules/renderer/system.hpp"
#include "modules/sprites/components.hpp"
#include "modules/sprites/system.hpp"
#include "modules/ui_editor_bar/components.hpp"
#include "modules/ui_editor_bar/system.hpp"
#include "modules/ui_editor_scene/system.hpp"
#include "modules/ui_editor_tilemap/components.hpp"
#include "modules/ui_editor_tilemap/system.hpp"
#include "modules/ui_hierarchy/components.hpp"
#include "modules/ui_hierarchy/helpers.hpp"
#include "modules/ui_hierarchy/system.hpp"
#include "modules/ui_networking/components.hpp"
#include "modules/ui_networking/system.hpp"
#include "modules/ui_physics/system.hpp"
#include "modules/ui_profiler/components.hpp"
#include "modules/ui_profiler/helpers.hpp"
#include "modules/ui_profiler/system.hpp"
#include "modules/ui_sprite_searcher/components.hpp"
#include "modules/ui_sprite_searcher/system.hpp"

#include "engine/app/application.hpp"
#include "game/components/components.hpp"
#include "game/entities/actors.hpp"
#include "game/simulate.hpp"
#include "game/systems/player.hpp"
#include "game/systems/player_inputs.hpp"
#include "game/systems/resolve_collisions.hpp"
#include "game/systems/ui_player_inventory.hpp"
#include "modules/camera/components.hpp"
#include "resources/audio.hpp"
#include "resources/colour.hpp"
#include "resources/textures.hpp"

// other lib
#include <glm/glm.hpp>

namespace game2d {

template<class T>
void
ctx_reset(entt::registry& r)
{
  if (r.ctx().contains<T>())
    r.ctx().erase<T>();
  r.ctx().emplace<T>();
};

void
init_game_state(entt::registry& r)
{
  r.each([&r](auto entity) { r.destroy(entity); });
  ctx_reset<SINGLETON_PhysicsComponent>(r);
  ctx_reset<SINGLETON_GameOverComponent>(r);
  ctx_reset<SINGLETON_EntityBinComponent>(r);
  ctx_reset<SINGLETON_FixedUpdateInputHistory>(r);

  // reset editor tools?
  auto tilemap_ent = r.create();
  r.emplace<TilemapComponent>(tilemap_ent);

  create_hierarchy_root_node(r);
  create_gameplay(r, ENTITY_TYPE::FREE_CURSOR);

  {
    auto shield = create_gameplay(r, ENTITY_TYPE::SHIELD);
    create_renderable(r, shield, ENTITY_TYPE::SHIELD);
    auto& shield_transform = r.get<TransformComponent>(shield);
    shield_transform.position.x = 0;
    shield_transform.position.y = 0;
    shield_transform.scale.x = 1;
    shield_transform.scale.y = 1;

    auto player = create_gameplay(r, ENTITY_TYPE::PLAYER);
    create_renderable(r, player, ENTITY_TYPE::PLAYER);
    auto& player_transform = r.get<TransformComponent>(player);
    player_transform.position.x = 200;
    player_transform.position.y = 200;

    { // set shield as child of player
      EntityHierarchyComponent& player_hierarchy = r.get<EntityHierarchyComponent>(player);
      EntityHierarchyComponent& shield_hierarchy = r.get<EntityHierarchyComponent>(shield);
      // remove the shield from the old parent
      EntityHierarchyComponent& parent_hierarchy = r.get<EntityHierarchyComponent>(shield_hierarchy.parent);
      for (int i = 0; i < parent_hierarchy.children.size(); i++) {
        if (parent_hierarchy.children[i] == shield) {
          parent_hierarchy.children.erase(parent_hierarchy.children.begin() + i);
          break;
        }
      }
      shield_hierarchy.parent = player;
      player_hierarchy.children.push_back(shield);
    }
  }

  {
    auto shopkeeper = create_gameplay(r, ENTITY_TYPE::SHOPKEEPER);
    create_renderable(r, shopkeeper, ENTITY_TYPE::SHOPKEEPER);
    auto& transform = r.get<TransformComponent>(shopkeeper);
    transform.position.x = 500;
    transform.position.y = 500;
  }

  // stock up!
  const auto& view = r.view<ShopKeeperComponent>();
  view.each([&r](auto shop_entity, auto& shopkeeper) {
    create_item(r, ENTITY_TYPE::POTION, shop_entity);
    create_item(r, ENTITY_TYPE::POTION, shop_entity);
    create_item(r, ENTITY_TYPE::POTION, shop_entity);
    create_item(r, ENTITY_TYPE::SWORD, shop_entity);
    create_item(r, ENTITY_TYPE::FIRE_SWORD, shop_entity);
    create_item(r, ENTITY_TYPE::SHIELD, shop_entity);
    create_item(r, ENTITY_TYPE::STONE, shop_entity);
    create_item(r, ENTITY_TYPE::CROSSBOW, shop_entity);
    create_item(r, ENTITY_TYPE::BOLT, shop_entity);
    create_item(r, ENTITY_TYPE::SCROLL_CONFUSION, shop_entity);
    create_item(r, ENTITY_TYPE::SCROLL_FIREBALL, shop_entity);
    create_item(r, ENTITY_TYPE::SCROLL_MAGIC_MISSILE, shop_entity);
  });
};

} // namespace game2d

void
game2d::init(entt::registry& r)
{
  const auto& app = r.ctx().at<engine::SINGLETON_Application>();

  // editor tools
  ctx_reset<Profiler>(r);
  ctx_reset<SINGLETON_EditorComponent>(r);
  ctx_reset<SINGLETON_SpriteSearcher>(r);
  // other
  ctx_reset<SINGLETON_Textures>(r);
  ctx_reset<SINGLETON_ColoursComponent>(r);
  init_sprite_system(r);
  init_render_system(r);
  init_input_system(r);
  init_audio_system(r);
  init_networking_system(r);

  // GOAL: remove init_game_state with the
  // map load/save functionality
  // eventually this game state should be savable to a file
  init_game_state(r);
};

void
game2d::fixed_update(entt::registry& r, uint64_t milliseconds_dt)
{
  auto& p = r.ctx().at<Profiler>();
  {
    auto _ = time_scope(&p, "(all)", true);
    auto& input = r.ctx().at<InputComponent>();
    auto& fixed_input = r.ctx().at<SINGLETON_FixedUpdateInputHistory>();

    // while offline, just clear out anything older than a tick
    // (until a gameplay system needs older input)
    fixed_input.history.clear();

    // move inputs from Update() to this FixedUpdate() tick
    fixed_input.history[fixed_input.fixed_tick] = std::move(input.unprocessed_update_inputs);
    auto& inputs = fixed_input.history[fixed_input.fixed_tick];

    simulate(r, inputs, milliseconds_dt);

    fixed_input.fixed_tick += 1;

    // update_networking_system(r, milliseconds_dt);
  }
}

void
game2d::update(entt::registry& r, float dt)
{
  auto& p = r.ctx().at<Profiler>();
  const auto& ri = r.ctx().at<SINGLETON_RendererInfo>();
  auto& app = r.ctx().at<engine::SINGLETON_Application>();
  {
    auto _ = time_scope(&p, "game_tick");
    update_input_system(r);
    auto& input = r.ctx().at<InputComponent>();
    if (ri.viewport_process_events) {
      if (get_key_down(input, SDL_SCANCODE_R))
        init_game_state(r);
      if (get_key_down(input, SDL_SCANCODE_F))
        app.window.toggle_fullscreen();
      if (get_key_down(input, SDL_SCANCODE_ESCAPE))
        app.running = false;
    }

    // ... systems that always update
    {
      // update_cursor_system(r);
      update_audio_system(r);
    }

    // ... systems that update only if viewport is focused or hovered
    {
      if (ri.viewport_process_events) {
        // update_camera_system(r);
        update_player_inputs_system(r);
        update_cursor_system(r);
      }
    }
  };
  {
    // put rendering on thread?
    auto _ = time_scope(&p, "rendering");
    update_sprite_system(r, dt);
    update_render_system(r);
  };
  {
    auto _ = time_scope(&p, "ui"); // value always be a frame behind
    {
      update_ui_player_inventory_system(r);
      update_ui_physics_system(r);
      update_ui_hierarchy_system(r);
      update_ui_profiler_system(r);
      update_ui_sprite_searcher_system(r);
      update_ui_editor_bar_system(r);
      update_ui_editor_tilemap_system(r);
      update_ui_editor_scene_system(r);
    }
    // update_ui_networking_system(r);
    // update_ui_main_menu_system(r);
  };

  // end frame
  {
    end_frame_render_system(r);
  }
};
