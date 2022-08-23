#include "actors.hpp"

#include "engine/maths/maths.hpp"
#include "game/components/components.hpp"
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

namespace game2d {

static constexpr int SPRITE_SIZE = 16;

entt::entity
create_item(entt::registry& r, const ENTITY_TYPE& type, const entt::entity& parent)
{
  auto e = create_entity(r, type);
  r.emplace<InBackpackComponent>(e, parent);
  return e;
};

void
create_renderable(entt::registry& r,
                  const entt::entity& e,
                  const ENTITY_TYPE& type,
                  const std::optional<engine::SRGBColour>& colour)
{
  const auto& slots = r.ctx().at<SINGLETON_Textures>();
  const auto& colours = r.ctx().at<SINGLETON_ColoursComponent>();
  const auto& sprites = r.ctx().at<SINGLETON_Animations>();

  TransformComponent t_comp;
  t_comp.scale.x = SPRITE_SIZE;
  t_comp.scale.y = SPRITE_SIZE;
  SpriteComponent s_comp;
  std::string sprite = "EMPTY";

  switch (type) {
    case ENTITY_TYPE::ENEMY: {
      s_comp.colour = engine::SRGBToLinear(colours.asteroid);
      s_comp.tex_unit = slots.tex_unit_kenny;
      // search kenny-nl spritesheet
      const auto anim = find_animation(sprites.animations, sprite);
      s_comp.x = anim.animation_frames[0].x;
      s_comp.y = anim.animation_frames[0].y;
      break;
    }
    case ENTITY_TYPE::PLAYER: {
      sprite = "PERSON_25_0";
      s_comp.colour = engine::SRGBToLinear(colours.player_unit);
      s_comp.tex_unit = slots.tex_unit_kenny;
      // search kenny-nl spritesheet
      const auto anim = find_animation(sprites.animations, sprite);
      s_comp.x = anim.animation_frames[0].x;
      s_comp.y = anim.animation_frames[0].y;
      break;
    }
    case ENTITY_TYPE::BOLT: {
      sprite = "PERSON_25_0";
      s_comp.colour = engine::SRGBToLinear(colours.bullet);
      s_comp.tex_unit = slots.tex_unit_kenny;
      // search kenny-nl spritesheet
      const auto anim = find_animation(sprites.animations, sprite);
      s_comp.x = anim.animation_frames[0].x;
      s_comp.y = anim.animation_frames[0].y;
      s_comp.angle_radians = anim.animation_angle_degrees * engine::PI / 180.0f;
      t_comp.scale.x = SPRITE_SIZE / 2;
      t_comp.scale.y = SPRITE_SIZE / 2;
      break;
    }
    case ENTITY_TYPE::POTION: {
      sprite = "DUCK";
      s_comp.colour = engine::SRGBToLinear(colours.bullet);
      s_comp.tex_unit = slots.tex_unit_kenny;
      // search kenny-nl spritesheet
      const auto anim = find_animation(sprites.animations, sprite);
      s_comp.x = anim.animation_frames[0].x;
      s_comp.y = anim.animation_frames[0].y;
      break;
    }
    case ENTITY_TYPE::SCROLL_MAGIC_MISSILE: {
      sprite = "ROCKET_1";
      s_comp.colour = engine::SRGBToLinear(colours.bullet);
      s_comp.tex_unit = slots.tex_unit_kenny;
      // s_comp.angle_radians = anim.animation_angle_degrees * engine::PI / 180.0f;
      // search kenny-nl spritesheet
      const auto anim = find_animation(sprites.animations, sprite);
      s_comp.x = anim.animation_frames[0].x;
      s_comp.y = anim.animation_frames[0].y;
      break;
    }
    case ENTITY_TYPE::SHOPKEEPER: {
      sprite = "PERSON_25_0";
      s_comp.colour = engine::SRGBToLinear(colours.bullet);
      s_comp.tex_unit = slots.tex_unit_kenny;
      // search kenny-nl spritesheet
      const auto anim = find_animation(sprites.animations, sprite);
      s_comp.x = anim.animation_frames[0].x;
      s_comp.y = anim.animation_frames[0].y;
      break;
    }
    case ENTITY_TYPE::GRID_CURSOR:
    case ENTITY_TYPE::FREE_CURSOR: {
      s_comp.colour = engine::SRGBToLinear(colours.red);
      s_comp.tex_unit = slots.tex_unit_kenny;
      break;
    }
    default: {
      std::cout << "(not implemented), tried to create renderable: " << std::string(magic_enum::enum_name(type))
                << std::endl;
    }
  }

  if (colour)
    s_comp.colour = engine::SRGBToLinear(colour.value());
  r.emplace_or_replace<SpriteComponent>(e, s_comp);
  r.emplace_or_replace<TransformComponent>(e, t_comp);
};

void
remove_renderable(entt::registry& r, const entt::entity& e)
{
  r.remove<SpriteComponent>(e);
  r.remove<TransformComponent>(e);
};

entt::entity
create_entity(entt::registry& r, const ENTITY_TYPE& type)
{
  const auto& h = r.ctx().at<SINGLETON_HierarchyComponent>();
  auto& hc = r.get<EntityHierarchyComponent>(h.root_node);
  auto e = r.create();
  hc.children.push_back(e);
  r.emplace<EntityHierarchyComponent>(e, h.root_node);
  r.emplace<TagComponent>(e, std::string(magic_enum::enum_name(type)));

  const auto& colours = r.ctx().at<SINGLETON_ColoursComponent>();

  switch (type) {
    case ENTITY_TYPE::ENEMY: {
      create_renderable(r, e, type);
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
      create_renderable(r, e, type);
      r.emplace<PhysicsActorComponent>(e, GameCollisionLayer::ACTOR_PLAYER);
      r.emplace<PhysicsSizeComponent>(e, PhysicsSizeComponent(SPRITE_SIZE, SPRITE_SIZE));
      r.emplace<VelocityComponent>(e);
      // gameplay
      r.emplace<HealthComponent>(e);
      r.emplace<TakeDamageComponent>(e);
      r.emplace<PlayerComponent>(e);
      break;
    }
    case ENTITY_TYPE::SHOPKEEPER: {
      create_renderable(r, e, type);
      // gameplay
      r.emplace<ShopKeeperComponent>(e);
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
      create_renderable(r, e, type);
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
      FreeCursorComponent c;
      c.line_u = r.create();
      create_renderable(r, c.line_u, ENTITY_TYPE::FREE_CURSOR);
      c.line_d = r.create();
      create_renderable(r, c.line_d, ENTITY_TYPE::FREE_CURSOR);
      c.line_l = r.create();
      create_renderable(r, c.line_l, ENTITY_TYPE::FREE_CURSOR);
      c.line_r = r.create();
      create_renderable(r, c.line_r, ENTITY_TYPE::FREE_CURSOR);
      c.backdrop = r.create();
      create_renderable(r, c.backdrop, ENTITY_TYPE::FREE_CURSOR, colours.backdrop_red);
      r.emplace<FreeCursorComponent>(e, c);
      // physics
      r.emplace<PhysicsActorComponent>(e, GameCollisionLayer::ACTOR_CURSOR);
      r.emplace<PhysicsSizeComponent>(e, PhysicsSizeComponent(SPRITE_SIZE, SPRITE_SIZE));
      break;
    }

    case ENTITY_TYPE::GRID_CURSOR: {
      GridCursorComponent c;
      c.line_u = r.create();
      create_renderable(r, c.line_u, ENTITY_TYPE::GRID_CURSOR, colours.cyan);
      c.line_d = r.create();
      create_renderable(r, c.line_d, ENTITY_TYPE::GRID_CURSOR, colours.cyan);
      c.line_l = r.create();
      create_renderable(r, c.line_l, ENTITY_TYPE::GRID_CURSOR, colours.cyan);
      c.line_r = r.create();
      create_renderable(r, c.line_r, ENTITY_TYPE::GRID_CURSOR, colours.cyan);
      c.backdrop = r.create();
      create_renderable(r, c.backdrop, ENTITY_TYPE::GRID_CURSOR, colours.dblue);
      r.emplace<GridCursorComponent>(e, c);
      // physics
      r.emplace<PhysicsActorComponent>(e, GameCollisionLayer::ACTOR_CURSOR);
      r.emplace<PhysicsSizeComponent>(e, PhysicsSizeComponent(SPRITE_SIZE, SPRITE_SIZE));
      break;
    }
  }

  return e;
};

} // namespace game2d