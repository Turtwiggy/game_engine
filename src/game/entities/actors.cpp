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
create_item(entt::registry& r, const ENTITY_TYPE& type, const entt::entity& parent)
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
create_sprite(entt::registry& r, const entt::entity& e, const ENTITY_TYPE& type)
{
  const auto& colours = r.ctx().at<SINGLETON_ColoursComponent>();
  const auto& sprites = r.ctx().at<SINGLETON_Animations>();
  const auto type_name = std::string(magic_enum::enum_name(type));

  SpriteComponent s_comp;
  s_comp.tex_unit = get_tex_unit(r, AvailableTexture::KENNY);
  std::string sprite = "EMPTY";

  // sprites
  if (type == ENTITY_TYPE::EMPTY)
    sprite = "EMPTY";
  else if (type == ENTITY_TYPE::WALL)
    sprite = "EMPTY";
  else if (type == ENTITY_TYPE::FLOOR)
    sprite = "EMPTY";
  else if (type == ENTITY_TYPE::AIM_LINE)
    sprite = "EMPTY";
  else if (type == ENTITY_TYPE::ENEMY)
    sprite = "PERSON_25_0";
  else if (type == ENTITY_TYPE::PLAYER)
    sprite = "PERSON_26_0";
  else if (type == ENTITY_TYPE::BOLT)
    sprite = "PERSON_25_0";
  else if (type == ENTITY_TYPE::SHIELD)
    sprite = "SHIELD_37_2";
  else if (type == ENTITY_TYPE::POTION)
    sprite = "DUCK";
  else if (type == ENTITY_TYPE::SCROLL_MAGIC_MISSILE)
    sprite = "ROCKET_1";
  else if (type == ENTITY_TYPE::SHOPKEEPER)
    sprite = "PERSON_25_0";
  else if (type == ENTITY_TYPE::FREE_CURSOR)
    sprite = "EMPTY";
  else
    std::cerr << "warning! sprite not implemented for: " << type_name << "\n";

  // colours
  if (type == ENTITY_TYPE::EMPTY)
    s_comp.colour = engine::SRGBToLinear(colours.white);
  else if (type == ENTITY_TYPE::WALL)
    s_comp.colour = engine::SRGBToLinear(colours.wall);
  else if (type == ENTITY_TYPE::FLOOR)
    s_comp.colour = engine::SRGBToLinear(colours.floor);
  else if (type == ENTITY_TYPE::AIM_LINE)
    s_comp.colour = engine::SRGBToLinear(colours.desat_red);
  else if (type == ENTITY_TYPE::ENEMY)
    s_comp.colour = engine::SRGBToLinear(colours.asteroid);
  else if (type == ENTITY_TYPE::PLAYER)
    s_comp.colour = engine::SRGBToLinear(colours.player_unit);
  else if (type == ENTITY_TYPE::BOLT)
    s_comp.colour = engine::SRGBToLinear(colours.bullet);
  else if (type == ENTITY_TYPE::SHIELD)
    s_comp.colour = engine::SRGBToLinear(colours.shield);
  else if (type == ENTITY_TYPE::POTION)
    s_comp.colour = engine::SRGBToLinear(colours.red);
  else if (type == ENTITY_TYPE::SCROLL_MAGIC_MISSILE)
    s_comp.colour = engine::SRGBToLinear(colours.red);
  else if (type == ENTITY_TYPE::SHOPKEEPER)
    s_comp.colour = engine::SRGBToLinear(colours.red);
  else if (type == ENTITY_TYPE::FREE_CURSOR)
    s_comp.colour = engine::SRGBToLinear(colours.red);
  else
    std::cerr << "warning! colour not implemented for: " << type_name << "\n";

  // search spritesheet
  const auto anim = find_animation(sprites.animations, sprite);
  s_comp.x = anim.animation_frames[0].x;
  s_comp.y = anim.animation_frames[0].y;

  // overwrite defaults for sprite
  // if (type == ENTITY_TYPE::BOLT) {
  //   s_comp.angle_radians = anim.animation_angle_degrees * engine::PI / 180.0f;
  //   t_comp.scale.x = SPRITE_SIZE / 2;
  //   t_comp.scale.y = SPRITE_SIZE / 2;
  // }

  // HACK
  if (s_comp.tex_unit == get_tex_unit(r, AvailableTexture::KENNY)) {
    s_comp.sx = 48;
    s_comp.sy = 22;
  }

  return s_comp;
};

void
create_renderable(entt::registry& r, const entt::entity& e, const ENTITY_TYPE& type)
{
  r.emplace<SpriteComponent>(e, create_sprite(r, e, type));
  r.emplace<TransformComponent>(e, create_transform(r, e));
};

entt::entity
create_gameplay(entt::registry& r, const ENTITY_TYPE& type)
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
create_gameplay(entt::registry& r, const entt::entity& e, const ENTITY_TYPE& type)
{
  const auto& colours = r.ctx().at<SINGLETON_ColoursComponent>();
  const auto type_name = std::string(magic_enum::enum_name(type));

  switch (type) {
    case ENTITY_TYPE::EMPTY: {
      break;
    }
    case ENTITY_TYPE::WALL: {
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
      //     sprite.colour = engine::SRGBToLinear(colours.red);
      //     sprite_tag.sprite = sprite;
      //   }
      //   ssc.sprites.push_back(sprite_tag);
      // }
      // r.emplace<SpriteSwapBasedOnStateComponent>(e, ssc);

      break;
    }
    case ENTITY_TYPE::FLOOR: {

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
      //     sprite.colour = engine::SRGBToLinear(colours.red);
      //     sprite_tag.sprite = sprite;
      //   }
      //   ssc.sprites.push_back(sprite_tag);
      // }
      // r.emplace<SpriteSwapBasedOnStateComponent>(e, ssc);

      break;
    }
    case ENTITY_TYPE::ENEMY: {
      // physics
      r.emplace<PhysicsActorComponent>(e, GameCollisionLayer::ACTOR_ENEMY);
      r.emplace<PhysicsSizeComponent>(e, PhysicsSizeComponent(SPRITE_SIZE, SPRITE_SIZE));
      r.emplace<VelocityComponent>(e);
      // gameplay
      r.emplace<HealthComponent>(e);
      r.emplace<TakeDamageComponent>(e);
      break;
    }
    case ENTITY_TYPE::PLAYER: {
      r.emplace<PhysicsActorComponent>(e, GameCollisionLayer::ACTOR_PLAYER);
      r.emplace<PhysicsSizeComponent>(e, PhysicsSizeComponent(SPRITE_SIZE, SPRITE_SIZE));
      // r.emplace<VelocityComponent>(e);
      r.emplace<GridMoveComponent>(e);
      // gameplay
      r.emplace<HealthComponent>(e);
      r.emplace<TakeDamageComponent>(e);

      PlayerComponent p;
      p.aim_line = create_gameplay(r, ENTITY_TYPE::AIM_LINE);
      create_renderable(r, p.aim_line, ENTITY_TYPE::AIM_LINE);

      r.emplace<PlayerComponent>(e, p);
      break;
    }
    case ENTITY_TYPE::SHOPKEEPER: {
      // gameplay
      r.emplace<ShopKeeperComponent>(e);
      break;
    }
    case ENTITY_TYPE::AIM_LINE: {
      break;
    }
    case ENTITY_TYPE::STONE: {
      r.emplace<AttackComponent>(e, AttackComponent(0, 4));
      break;
    }
    case ENTITY_TYPE::SWORD: {
      r.emplace<AttackComponent>(e, AttackComponent(10, 20));
      break;
    }
    case ENTITY_TYPE::FIRE_SWORD: {
      r.emplace<AttackComponent>(e, AttackComponent(30, 40));
      break;
    }
    case ENTITY_TYPE::CROSSBOW: {
      r.emplace<AttackComponent>(e, AttackComponent(10, 20));
      break;
    }
    case ENTITY_TYPE::BOLT: {
      r.emplace<PhysicsActorComponent>(e, GameCollisionLayer::ACTOR_BULLET);
      r.emplace<PhysicsSizeComponent>(e, PhysicsSizeComponent(SPRITE_SIZE / 2, SPRITE_SIZE / 2));
      r.emplace<VelocityComponent>(e);
      // gameplay
      // r.emplace<EntityTimedLifecycle>(e, 20000); // bullet time alive
      break;
    }
    case ENTITY_TYPE::SHIELD: {
      r.emplace<AttackComponent>(e, AttackComponent(5, 8));
      r.emplace<DefenseComponent>(e, DefenseComponent(10));
      break;
    }

      // consumable items

    case ENTITY_TYPE::POTION: {
      r.emplace<ConsumableComponent>(e);
      r.emplace<GiveHealsComponent>(e);
      break;
    }
    case ENTITY_TYPE::SCROLL_MAGIC_MISSILE: {
      r.emplace<ConsumableComponent>(e);
      r.emplace<AttackComponent>(e, AttackComponent(5, 10));
      r.emplace<RangedComponent>(e);
      // r.emplace<GiveDamageComponent>(e);
      break;
    }
    case ENTITY_TYPE::SCROLL_FIREBALL: {
      r.emplace<ConsumableComponent>(e);
      r.emplace<AttackComponent>(e, AttackComponent(5, 10));
      r.emplace<RangedComponent>(e);
      // r.emplace<GiveDamageComponent>(e);
      // r.emplace<AreaOfEffectComponent>(e);
      break;
    }
    case ENTITY_TYPE::SCROLL_CONFUSION: {
      r.emplace<ConsumableComponent>(e);
      r.emplace<RangedComponent>(e);
      // r.emplace<ConfusionComponent>(e);
      break;
    }

      // misc...

    case ENTITY_TYPE::FREE_CURSOR: {

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