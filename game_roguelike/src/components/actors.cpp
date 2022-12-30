#include "actors.hpp"

#include "audio/components.hpp"
#include "camera/components.hpp"
#include "colour/colour.hpp"
#include "lifecycle/components.hpp"
#include "maths/maths.hpp"
#include "modules/ai/components.hpp"
#include "modules/combat/components.hpp"
#include "modules/cursor/components.hpp"
#include "modules/items/components.hpp"
#include "modules/player/components.hpp"
#include "modules/rpg_xp/components.hpp"
#include "physics/components.hpp"
#include "renderer/components.hpp"
#include "sprites/components.hpp"
#include "sprites/helpers.hpp"
#include "ui_hierarchy/components.hpp"

#include "magic_enum.hpp"

#include <iostream>

namespace game2d {

static constexpr int SPRITE_SIZE = 16;

void
to_json(json& j, const EntityTypeComponent& et)
{
  j = json{ { "type", static_cast<int>(et.type) } };
};

void
from_json(const json& j, EntityTypeComponent& et)
{
  j.at("type").get_to(et.type);
};

void
set_parent(entt::registry& r, const entt::entity& e, const entt::entity& parent)
{
  r.emplace<EntityHierarchyComponent>(e, parent);
};

void
add_child(entt::registry& r, const entt::entity& e, const entt::entity& child)
{
  auto& hc = r.get<EntityHierarchyComponent>(e);
  hc.children.push_back(child);
};

TransformComponent
create_transform()
{
  TransformComponent t_comp;
  t_comp.scale.x = SPRITE_SIZE;
  t_comp.scale.y = SPRITE_SIZE;
  return t_comp;
};

SpriteComponent
create_sprite(GameEditor& editor, const EntityType& type)
{
  const auto& sprites = editor.animations;
  const auto& textures = editor.textures;
  const auto type_name = std::string(magic_enum::enum_name(type));

  std::string sprite = "EMPTY";
  if (type == EntityType::empty)
    sprite = "EMPTY";
  else if (type == EntityType::tile_type_wall)
    sprite = "TREE_0_1";
  else if (type == EntityType::tile_type_exit)
    sprite = "DOOR_4_13";
  else if (type == EntityType::tile_type_floor)
    sprite = "CASTLE_FLOOR";
  else if (type == EntityType::actor_shopkeeper)
    sprite = "PERSON_25_0";
  else if (type == EntityType::actor_bat)
    sprite = "BAT_26_8";
  else if (type == EntityType::actor_troll)
    sprite = "PERSON_29_2";
  else if (type == EntityType::actor_player)
    sprite = "PERSON_25_0";
  // else if (type == EntityType::bolt)
  //   sprite = "DART_35_5";
  // else if (type == EntityType::crossbow)
  //   sprite = "CROSSBOW_37_5";
  else if (type == EntityType::sword)
    sprite = "SWORD_32_7";
  else if (type == EntityType::shield)
    sprite = "SHIELD_37_2";
  else if (type == EntityType::potion)
    sprite = "FULL_POTION_34_13";
  else if (type == EntityType::scroll_damage_nearest)
    sprite = "SCROLL_33_5";
  // else if (type == EntityType::scroll_damage_selected_on_grid)
  //   sprite = "SCROLL_34_5";
  else if (type == EntityType::free_cursor)
    sprite = "EMPTY";
  else if (type == EntityType::grid_cursor)
    sprite = "EMPTY";
  else
    std::cerr << "warning! renderable not implemented: " << type_name << "\n";

  RenderOrder order = RenderOrder::background;

  if (type == EntityType::actor_bat)
    order = RenderOrder::foreground;
  else if (type == EntityType::actor_troll)
    order = RenderOrder::foreground;
  else if (type == EntityType::actor_player)
    order = RenderOrder::foreground;
  // else if (type == EntityType::bolt)
  //   order = RenderOrder::foreground;
  else if (type == EntityType::shield)
    order = RenderOrder::foreground;
  else if (type == EntityType::potion)
    order = RenderOrder::foreground;
  else if (type == EntityType::scroll_damage_nearest)
    order = RenderOrder::foreground;
  // else if (type == EntityType::scroll_damage_selected_on_grid)
  //   order = RenderOrder::foreground;

  SpriteComponent sc;
  sc.render_order = order;

  // search spritesheet
  const auto anim = find_animation(sprites.animations, sprite);
  sc.x = anim.animation_frames[0].x;
  sc.y = anim.animation_frames[0].y;

  // limitation: all sprites are now kenny sprites
  sc.tex_unit = get_tex_unit(textures, AvailableTexture::kenny);
  if (sc.tex_unit == get_tex_unit(textures, AvailableTexture::kenny)) {
    sc.sx = 48;
    sc.sy = 22;
  }

  return sc;
};

SpriteColourComponent
create_colour(GameEditor& editor, const EntityType& type)
{
  const auto type_name = std::string(magic_enum::enum_name(type));
  const auto& primary = editor.colours.primary;
  const auto& secondary = editor.colours.secondary;
  const auto& tertiary = editor.colours.tertiary;
  const auto& quaternary = editor.colours.quaternary;

  engine::SRGBColour srgb = primary;
  if (type == EntityType::empty)
    srgb = primary;
  else if (type == EntityType::tile_type_wall)
    srgb = secondary;
  else if (type == EntityType::tile_type_floor)
    srgb = secondary;
  else if (type == EntityType::tile_type_exit)
    srgb = primary;
  else if (type == EntityType::actor_bat)
    srgb = primary;
  else if (type == EntityType::actor_troll)
    srgb = primary;
  else if (type == EntityType::actor_player)
    srgb = primary;
  // else if (type == EntityType::bolt)
  //   srgb = colours.lightyellow;
  // else if (type == EntityType::crossbow)
  //   srgb = colours.red;
  else if (type == EntityType::sword)
    srgb = primary;
  else if (type == EntityType::shield)
    srgb = primary;
  else if (type == EntityType::potion)
    srgb = primary;
  else if (type == EntityType::scroll_damage_nearest)
    srgb = primary;
  else if (type == EntityType::actor_shopkeeper)
    srgb = primary;
  else if (type == EntityType::free_cursor)
    srgb = primary;
  else if (type == EntityType::grid_cursor)
    srgb = primary;
  else
    std::cerr << "warning! colour not chosen for: " << type_name << "\n";

  SpriteColourComponent scc;
  scc.colour = engine::SRGBToLinear(srgb);
  return scc;
}

AudioSource
create_audio(GameEditor& editor, const EntityType& type)
{
  AudioSource source;

  if (type == EntityType::empty) {
    source.audio_selection = {};
  } else if (type == EntityType::actor_player) {
    source.audio_selection = { AvailableSound::player_got_hit };
  } else if (type == EntityType::actor_bat) {
    source.audio_selection = { AvailableSound::player_attacked };
  }

  return source;
}

void
create_renderable(GameEditor& editor, Game& g, const entt::entity& e, const EntityType& type)
{
  create_renderable(editor, g.state, e, type);
};

void
create_renderable(GameEditor& editor, entt::registry& r, const entt::entity& e, const EntityType& type)
{
  r.emplace<SpriteComponent>(e, create_sprite(editor, type));
  r.emplace<SpriteColourComponent>(e, create_colour(editor, type));
  r.emplace<TransformComponent>(e, create_transform());
};

entt::entity
create_item(GameEditor& editor, entt::registry& r, const EntityType& type, const entt::entity& parent)
{
  auto e = create_gameplay(editor, r, type);
  r.emplace<InBackpackComponent>(e, parent);
  return e;
};

entt::entity
create_item(GameEditor& editor, Game& game, const EntityType& type, const entt::entity& parent)
{
  return create_item(editor, game.state, type, parent);
};

entt::entity
create_gameplay(GameEditor& editor, Game& g, const EntityType& type)
{
  return create_gameplay(editor, g.state, type);
};

entt::entity
create_gameplay(GameEditor& editor, entt::registry& r, const EntityType& type)
{
  const auto& h = r.view<RootNode>().front();
  const auto& e = r.create();

  add_child(r, h, e);
  set_parent(r, e, h);
  r.emplace<TagComponent>(e, std::string(magic_enum::enum_name(type)));
  r.emplace<EntityTypeComponent>(e, type);

  create_gameplay_existing_entity(editor, r, e, type);

  return e;
};

void
create_gameplay_existing_entity(GameEditor& editor, Game& g, const entt::entity& e, const EntityType& type)
{
  create_gameplay_existing_entity(editor, g.state, e, type);
};

void
create_gameplay_existing_entity(GameEditor& editor, entt::registry& r, const entt::entity& e, const EntityType& type)
{
  const auto& colours = editor.colours;
  const auto type_name = std::string(magic_enum::enum_name(type));

  switch (type) {
    case EntityType::empty: {
      break;
    }
    case EntityType::tile_type_wall: {
      // physics
      r.emplace<PhysicsTransformComponent>(e);
      r.emplace<PhysicsSolidComponent>(e);
      break;
    }
    case EntityType::tile_type_floor: {
      // gameplay
      break;
    }
    case EntityType::tile_type_exit: {
      // physics
      r.emplace<PhysicsTransformComponent>(e);
      r.emplace<PhysicsActorComponent>(e);
      // gameplay
      break;
    }

      // pc/npcs

    case EntityType::actor_bat: {
      // physics
      r.emplace<PhysicsTransformComponent>(e);
      r.emplace<PhysicsActorComponent>(e);
      r.emplace<PhysicsSolidComponent>(e);
      r.emplace<GridMoveComponent>(e);
      // gameplay
      r.emplace<AudioSource>(e, create_audio(editor, type));
      r.emplace<AiBrainComponent>(e);
      r.emplace<HealthComponent>(e, 3);
      r.emplace<TakeDamageComponent>(e);
      r.emplace<XpComponent>(e, 50);

      StatsComponent stats;
      stats.con_level = 2;
      stats.str_level = 2;
      stats.agi_level = 0;
      r.emplace<StatsComponent>(e, stats);
      break;
    }
    case EntityType::actor_troll: {
      // physics
      r.emplace<PhysicsTransformComponent>(e);
      r.emplace<PhysicsActorComponent>(e);
      r.emplace<PhysicsSolidComponent>(e);
      r.emplace<GridMoveComponent>(e);
      // gameplay
      r.emplace<AudioSource>(e, create_audio(editor, type));
      r.emplace<AiBrainComponent>(e);
      r.emplace<HealthComponent>(e, 10);
      r.emplace<TakeDamageComponent>(e);
      r.emplace<XpComponent>(e, 100);

      StatsComponent stats;
      stats.con_level = 4;
      stats.str_level = 4;
      stats.agi_level = 1;
      r.emplace<StatsComponent>(e, stats);
      break;
    }
    case EntityType::actor_player: {
      r.emplace<PhysicsTransformComponent>(e);
      r.emplace<PhysicsActorComponent>(e);
      r.emplace<GridMoveComponent>(e);
      // gameplay
      r.emplace<AudioSource>(e, create_audio(editor, type));
      r.emplace<PlayerComponent>(e);
      r.emplace<HealthComponent>(e);
      r.emplace<TakeDamageComponent>(e);
      r.emplace<XpComponent>(e, 0);

      StatsComponent stats;
      stats.con_level = 1;
      stats.agi_level = 1;
      stats.str_level = 1;
      r.emplace<StatsComponent>(e, stats);
      break;
    }
    case EntityType::actor_shopkeeper: {
      // gameplay
      r.emplace<ShopKeeperComponent>(e);
      break;
    }

      // items

    // case EntityType::bolt: {
    //   r.emplace<PhysicsTransformComponent>(e);
    //   r.emplace<PhysicsActorComponent>(e);
    //   r.emplace<VelocityComponent>(e);
    //   r.emplace<AbleToBePickedUp>(e);
    //   r.emplace<ThrowableComponent>(e);
    //   // gameplay
    //   // r.emplace<EntityTimedLifecycle>(e, 20000); // bullet time alive
    //   break;
    // }
    // case EntityType::crossbow: {
    //   r.emplace<PhysicsTransformComponent>(e);
    //   r.emplace<PhysicsActorComponent>(e);
    //   r.emplace<Equipment>(e);
    //   r.emplace<AbleToBePickedUp>(e);
    //   r.emplace<AttackComponent>(e, AttackComponent(5, 10));
    //   break;
    // }
    case EntityType::sword: {
      r.emplace<PhysicsTransformComponent>(e);
      r.emplace<PhysicsActorComponent>(e);
      r.emplace<Equipment>(e);
      r.emplace<AbleToBePickedUp>(e);
      r.emplace<AttackComponent>(e, AttackComponent(1, 4));
      break;
    }
    case EntityType::shield: {
      r.emplace<PhysicsTransformComponent>(e);
      r.emplace<PhysicsActorComponent>(e);
      r.emplace<Equipment>(e);
      r.emplace<AbleToBePickedUp>(e);
      r.emplace<AttackComponent>(e, AttackComponent(5, 8));
      r.emplace<DefenseComponent>(e, DefenseComponent(1));
      break;
    }
    case EntityType::potion: {
      r.emplace<PhysicsTransformComponent>(e);
      r.emplace<PhysicsActorComponent>(e);
      r.emplace<ConsumableComponent>(e);
      r.emplace<AbleToBePickedUp>(e);
      r.emplace<GiveHealsComponent>(e);
      break;
    }
    case EntityType::scroll_damage_nearest: {
      r.emplace<PhysicsTransformComponent>(e);
      r.emplace<PhysicsActorComponent>(e);
      r.emplace<ConsumableComponent>(e);
      r.emplace<AbleToBePickedUp>(e);
      r.emplace<AttackComponent>(e, AttackComponent(5, 10));
      break;
    }
      // case EntityType::scroll_damage_selected_on_grid: {
      //   r.emplace<PhysicsTransformComponent>(e);
      //   r.emplace<PhysicsActorComponent>(e);
      //   r.emplace<ConsumableComponent>(e);
      //   r.emplace<AbleToBePickedUp>(e);
      //   r.emplace<AttackComponent>(e, AttackComponent(5, 10));
      //   break;
      // }

      // misc...
    case EntityType::camera: {
      r.emplace<CameraComponent>(e);
      r.emplace<TransformComponent>(e);
      break;
    }

    case EntityType::free_cursor: {
      const auto& h = r.view<RootNode>().front();

      auto create = [&r, &editor, &h, &type](const std::string& name) {
        auto line = r.create();
        r.emplace<TagComponent>(line, name);
        r.emplace<EntityTypeComponent>(line, type);
        create_gameplay_existing_entity(editor, r, line, EntityType::empty);
        create_renderable(editor, r, line, EntityType::free_cursor);
        add_child(r, h, line);
        set_parent(r, line, h);
        return line;
      };

      auto line_u = create("line_u");
      auto line_d = create("line_d");
      auto line_l = create("line_l");
      auto line_r = create("line_r");
      auto backdrop = create("backdrop");
      r.emplace<PhysicsActorComponent>(backdrop);
      r.emplace<PhysicsTransformComponent>(backdrop);

      FreeCursorComponent c;
      c.line_u = line_u;
      c.line_d = line_d;
      c.line_l = line_l;
      c.line_r = line_r;
      c.backdrop = backdrop;
      r.emplace<FreeCursorComponent>(e, c);
      break;
    }

    default: {
      std::cout << "warning: no gameplay implemented for: " << type_name;
    }
  }
};

} // namespace game2d