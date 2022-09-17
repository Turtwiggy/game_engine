#include "actors.hpp"

#include "engine/maths/maths.hpp"
#include "game/components/components.hpp"
#include "game/helpers/dungeon.hpp"
#include "modules/cursor/components.hpp"
#include "modules/lifecycle/components.hpp"
#include "modules/physics/components.hpp"
#include "modules/renderer/components.hpp"
#include "modules/sprites/components.hpp"
#include "modules/sprites/helpers.hpp"
#include "modules/ui_hierarchy/components.hpp"
#include "resources/colour.hpp"
#include "resources/textures.hpp"

#include "magic_enum.hpp"

#include <iostream>

namespace game2d {

static constexpr int SPRITE_SIZE = 16;

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

entt::entity
create_item(entt::registry& r, const EntityType& type, const entt::entity& parent)
{
  auto e = r.create();

  const auto& h = r.view<RootNode>().front();
  add_child(r, h, e);
  set_parent(r, e, h);
  r.emplace<TagComponent>(e, std::string(magic_enum::enum_name(type)));

  create_gameplay(r, e, type);

  r.emplace<InBackpackComponent>(e, parent);
  return e;
};

TransformComponent
create_transform(entt::registry& r, const entt::entity& e)
{
  TransformComponent t_comp;
  t_comp.scale.x = SPRITE_SIZE;
  t_comp.scale.y = SPRITE_SIZE;
  return t_comp;
};

SpriteComponent
create_sprite(entt::registry& r, const entt::entity& e, const EntityType& type)
{
  const auto& sprites = r.ctx().at<SINGLETON_Animations>();
  const auto type_name = std::string(magic_enum::enum_name(type));

  std::string sprite = "EMPTY";
  if (type == EntityType::empty)
    sprite = "EMPTY";
  else if (type == EntityType::wall)
    sprite = "EMPTY";
  else if (type == EntityType::floor)
    sprite = "EMPTY";
  else if (type == EntityType::aim_line)
    sprite = "EMPTY";
  else if (type == EntityType::enemy)
    sprite = "PERSON_25_0";
  else if (type == EntityType::player)
    sprite = "PERSON_26_0";
  else if (type == EntityType::bolt)
    sprite = "PERSON_25_0";
  else if (type == EntityType::shield)
    sprite = "SHIELD_37_2";
  else if (type == EntityType::potion)
    sprite = "DUCK";
  else if (type == EntityType::scroll_magic_missile)
    sprite = "ROCKET_1";
  else if (type == EntityType::shopkeeper)
    sprite = "PERSON_25_0";
  else if (type == EntityType::free_cursor)
    sprite = "EMPTY";
  else
    std::cerr << "warning! not renderable: " << type_name << "\n";

  SpriteComponent sc;

  // search spritesheet
  const auto anim = find_animation(sprites.animations, sprite);
  sc.x = anim.animation_frames[0].x;
  sc.y = anim.animation_frames[0].y;

  // limitation: all sprites are now kenny sprites
  sc.tex_unit = get_tex_unit(r, AvailableTexture::kenny);
  if (sc.tex_unit == get_tex_unit(r, AvailableTexture::kenny)) {
    sc.sx = 48;
    sc.sy = 22;
  }

  return sc;
};

SpriteColourComponent
create_colour(entt::registry& r, const entt::entity& e, const EntityType& type)
{
  const auto& colours = r.ctx().at<SINGLETON_ColoursComponent>();
  const auto type_name = std::string(magic_enum::enum_name(type));

  engine::SRGBColour srgb = colours.white;
  if (type == EntityType::empty)
    srgb = colours.white;
  else if (type == EntityType::wall)
    srgb = colours.wall;
  else if (type == EntityType::floor)
    srgb = colours.floor;
  else if (type == EntityType::aim_line)
    srgb = colours.desat_red;
  else if (type == EntityType::enemy)
    srgb = colours.asteroid;
  else if (type == EntityType::player)
    srgb = colours.player_unit;
  else if (type == EntityType::bolt)
    srgb = colours.bullet;
  else if (type == EntityType::shield)
    srgb = colours.shield;
  else if (type == EntityType::potion)
    srgb = colours.red;
  else if (type == EntityType::scroll_magic_missile)
    srgb = colours.red;
  else if (type == EntityType::shopkeeper)
    srgb = colours.red;
  else if (type == EntityType::free_cursor)
    srgb = colours.red;
  else
    std::cerr << "warning! not renderable: " << type_name << "\n";

  SpriteColourComponent scc;
  scc.colour = engine::SRGBToLinear(srgb);
  return scc;
}

void
create_renderable(entt::registry& r, const entt::entity& e, const EntityType& type)
{
  r.emplace<SpriteComponent>(e, create_sprite(r, e, type));
  r.emplace<SpriteColourComponent>(e, create_colour(r, e, type));
  r.emplace<TransformComponent>(e, create_transform(r, e));
};

entt::entity
create_gameplay(entt::registry& r, const EntityType& type)
{
  const auto& h = r.view<RootNode>().front();
  const auto& e = r.create();

  add_child(r, h, e);
  set_parent(r, e, h);
  r.emplace<TagComponent>(e, std::string(magic_enum::enum_name(type)));

  create_gameplay(r, e, type);

  return e;
};

void
create_gameplay(entt::registry& r, const entt::entity& e, const EntityType& type)
{
  const auto& colours = r.ctx().at<SINGLETON_ColoursComponent>();
  const auto type_name = std::string(magic_enum::enum_name(type));

  switch (type) {
    case EntityType::empty: {
      break;
    }
    case EntityType::wall: {
      // physics
      r.emplace<PhysicsSolidComponent>(e, GameCollisionLayer::SOLID_WALL);
      r.emplace<PhysicsSizeComponent>(e, PhysicsSizeComponent(SPRITE_SIZE, SPRITE_SIZE));
      // gameplay
      // r.emplace<TileBlocksFoVComponent>(e);

      // SpriteSwapBasedOnStateComponent ssc;
      // {
      //   SpriteTagComponent sprite_tag;
      //   sprite_tag.tag = "default";
      //   {
      //     SpriteComponent sprite = create_sprite(r, e, type);
      //     sprite_tag.sprite = sprite;
      //   }
      //   ssc.sprites.push_back(sprite_tag);
      // }
      // {
      //   SpriteTagComponent sprite_tag;
      //   sprite_tag.tag = "out_of_view";
      //   {
      //     SpriteComponent sprite;
      //     sprite.srgb= colours.red;
      //     sprite_tag.sprite = sprite;
      //   }
      //   ssc.sprites.push_back(sprite_tag);
      // }
      // r.emplace<SpriteSwapBasedOnStateComponent>(e, ssc);

      break;
    }
    case EntityType::floor: {

      // gameplay

      // SpriteSwapBasedOnStateComponent ssc;
      // {
      //   SpriteTagComponent sprite_tag;
      //   sprite_tag.tag = "default";
      //   {
      //     SpriteComponent sprite = create_sprite(r, e, type);
      //     sprite_tag.sprite = sprite;
      //   }
      //   ssc.sprites.push_back(sprite_tag);
      // }
      // {
      //   SpriteTagComponent sprite_tag;
      //   sprite_tag.tag = "out_of_view";
      //   {
      //     SpriteComponent sprite;
      //     sprite.srgb= colours.red;
      //     sprite_tag.sprite = sprite;
      //   }
      //   ssc.sprites.push_back(sprite_tag);
      // }
      // r.emplace<SpriteSwapBasedOnStateComponent>(e, ssc);

      break;
    }
    case EntityType::enemy: {
      // physics
      r.emplace<PhysicsActorComponent>(e, GameCollisionLayer::ACTOR_ENEMY);
      r.emplace<PhysicsSizeComponent>(e, PhysicsSizeComponent(SPRITE_SIZE, SPRITE_SIZE));
      r.emplace<VelocityComponent>(e);
      // gameplay
      r.emplace<HealthComponent>(e);
      r.emplace<TakeDamageComponent>(e);
      break;
    }
    case EntityType::player: {
      r.emplace<PhysicsActorComponent>(e, GameCollisionLayer::ACTOR_PLAYER);
      r.emplace<PhysicsSizeComponent>(e, PhysicsSizeComponent(SPRITE_SIZE, SPRITE_SIZE));
      // r.emplace<VelocityComponent>(e);
      r.emplace<GridMoveComponent>(e);
      // gameplay
      r.emplace<HealthComponent>(e);
      r.emplace<TakeDamageComponent>(e);

      PlayerComponent p;
      p.aim_line = create_gameplay(r, EntityType::aim_line);
      create_renderable(r, p.aim_line, EntityType::aim_line);

      r.emplace<PlayerComponent>(e, p);
      break;
    }
    case EntityType::shopkeeper: {
      // gameplay
      r.emplace<ShopKeeperComponent>(e);
      break;
    }
    case EntityType::aim_line: {
      break;
    }
    case EntityType::stone: {
      r.emplace<AttackComponent>(e, AttackComponent(0, 4));
      break;
    }
    case EntityType::sword: {
      r.emplace<AttackComponent>(e, AttackComponent(10, 20));
      break;
    }
    case EntityType::fire_sword: {
      r.emplace<AttackComponent>(e, AttackComponent(30, 40));
      break;
    }
    case EntityType::crossbow: {
      r.emplace<AttackComponent>(e, AttackComponent(10, 20));
      break;
    }
    case EntityType::bolt: {
      r.emplace<PhysicsActorComponent>(e, GameCollisionLayer::ACTOR_BULLET);
      r.emplace<PhysicsSizeComponent>(e, PhysicsSizeComponent(SPRITE_SIZE / 2, SPRITE_SIZE / 2));
      r.emplace<VelocityComponent>(e);
      // gameplay
      // r.emplace<EntityTimedLifecycle>(e, 20000); // bullet time alive
      break;
    }
    case EntityType::shield: {
      r.emplace<AttackComponent>(e, AttackComponent(5, 8));
      r.emplace<DefenseComponent>(e, DefenseComponent(10));
      break;
    }

      // consumable items

    case EntityType::potion: {
      r.emplace<ConsumableComponent>(e);
      r.emplace<GiveHealsComponent>(e);
      break;
    }
    case EntityType::scroll_magic_missile: {
      r.emplace<ConsumableComponent>(e);
      r.emplace<AttackComponent>(e, AttackComponent(5, 10));
      r.emplace<RangedComponent>(e);
      // r.emplace<GiveDamageComponent>(e);
      break;
    }
    case EntityType::scroll_fireball: {
      r.emplace<ConsumableComponent>(e);
      r.emplace<AttackComponent>(e, AttackComponent(5, 10));
      r.emplace<RangedComponent>(e);
      // r.emplace<GiveDamageComponent>(e);
      // r.emplace<AreaOfEffectComponent>(e);
      break;
    }
    case EntityType::scroll_confusion: {
      r.emplace<ConsumableComponent>(e);
      r.emplace<RangedComponent>(e);
      // r.emplace<ConfusionComponent>(e);
      break;
    }

      // misc...

    case EntityType::free_cursor: {

      // auto line_u = r.create();
      // r.emplace<TagComponent>(line_u, "line-u");
      // add_child(r, h, line_u);
      // set_parent(r, line_u, h);

      // auto line_d = r.create();
      // r.emplace<TagComponent>(line_d, "line-d");
      // add_child(r, h, line_d);
      // set_parent(r, line_d, h);

      // auto line_l = r.create();
      // r.emplace<TagComponent>(line_l, "line-l");
      // add_child(r, h, line_l);
      // set_parent(r, line_l, h);

      // auto line_r = r.create();
      // r.emplace<TagComponent>(line_r, "line-r");
      // add_child(r, h, line_r);
      // set_parent(r, line_r, h);

      // auto backdrop = r.create();
      // r.emplace<TagComponent>(backdrop, "backdrop");
      // add_child(r, h, backdrop);
      // set_parent(r, backdrop, h);

      // // object tag
      // FreeCursorComponent c;
      // c.line_u = line_u;
      // c.line_d = line_d;
      // c.line_l = line_l;
      // c.line_r = line_r;
      // c.backdrop = backdrop;
      // r.emplace<FreeCursorComponent>(e, c);
      // // physics
      // r.emplace<PhysicsActorComponent>(e, GameCollisionLayer::ACTOR_CURSOR);
      // r.emplace<PhysicsSizeComponent>(e, PhysicsSizeComponent(SPRITE_SIZE, SPRITE_SIZE));
      break;
    }

    default: {
      std::cout << "warning: no gameplay implemented for: " << type_name;
    }
  }
};

} // namespace game2d