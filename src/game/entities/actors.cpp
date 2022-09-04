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
      s_comp.tex_unit = get_tex_unit(r, AvailableTexture::KENNY);
      // search kenny-nl spritesheet
      const auto anim = find_animation(sprites.animations, sprite);
      s_comp.x = anim.animation_frames[0].x;
      s_comp.y = anim.animation_frames[0].y;
      break;
    }
    case ENTITY_TYPE::PLAYER: {
      sprite = "PERSON_25_0";
      s_comp.colour = engine::SRGBToLinear(colours.player_unit);
      s_comp.tex_unit = get_tex_unit(r, AvailableTexture::KENNY);
      // search kenny-nl spritesheet
      const auto anim = find_animation(sprites.animations, sprite);
      s_comp.x = anim.animation_frames[0].x;
      s_comp.y = anim.animation_frames[0].y;
      break;
    }
    case ENTITY_TYPE::BOLT: {
      sprite = "PERSON_25_0";
      s_comp.colour = engine::SRGBToLinear(colours.bullet);
      s_comp.tex_unit = get_tex_unit(r, AvailableTexture::KENNY);
      // search kenny-nl spritesheet
      const auto anim = find_animation(sprites.animations, sprite);
      s_comp.x = anim.animation_frames[0].x;
      s_comp.y = anim.animation_frames[0].y;
      s_comp.angle_radians = anim.animation_angle_degrees * engine::PI / 180.0f;
      t_comp.scale.x = SPRITE_SIZE / 2;
      t_comp.scale.y = SPRITE_SIZE / 2;
      break;
    }
    case ENTITY_TYPE::SHIELD: {
      sprite = "SHIELD_37_2";
      s_comp.colour = engine::SRGBToLinear(colours.shield);
      s_comp.tex_unit = get_tex_unit(r, AvailableTexture::KENNY);
      // search kenny-nl spritesheet
      const auto anim = find_animation(sprites.animations, sprite);
      s_comp.x = anim.animation_frames[0].x;
      s_comp.y = anim.animation_frames[0].y;
      break;
    }
    case ENTITY_TYPE::POTION: {
      sprite = "DUCK";
      s_comp.colour = engine::SRGBToLinear(colours.bullet);
      s_comp.tex_unit = get_tex_unit(r, AvailableTexture::KENNY);
      // search kenny-nl spritesheet
      const auto anim = find_animation(sprites.animations, sprite);
      s_comp.x = anim.animation_frames[0].x;
      s_comp.y = anim.animation_frames[0].y;
      break;
    }
    case ENTITY_TYPE::SCROLL_MAGIC_MISSILE: {
      sprite = "ROCKET_1";
      s_comp.colour = engine::SRGBToLinear(colours.bullet);
      s_comp.tex_unit = get_tex_unit(r, AvailableTexture::KENNY);
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
      s_comp.tex_unit = get_tex_unit(r, AvailableTexture::KENNY);
      // search kenny-nl spritesheet
      const auto anim = find_animation(sprites.animations, sprite);
      s_comp.x = anim.animation_frames[0].x;
      s_comp.y = anim.animation_frames[0].y;
      break;
    }
    case ENTITY_TYPE::GRID_CURSOR:
    case ENTITY_TYPE::FREE_CURSOR: {
      s_comp.colour = engine::SRGBToLinear(colours.red);
      s_comp.tex_unit = get_tex_unit(r, AvailableTexture::KENNY);
      break;
    }
    default: {
      std::cout << "(not implemented), tried to create renderable: " << std::string(magic_enum::enum_name(type))
                << "\n";
    }
  }

  if (colour)
    s_comp.colour = engine::SRGBToLinear(colour.value());

  // HACK
  if (s_comp.tex_unit == get_tex_unit(r, AvailableTexture::KENNY)) {
    s_comp.sx = 48;
    s_comp.sy = 22;
  }

  r.emplace_or_replace<SpriteComponent>(e, s_comp);
  r.emplace_or_replace<TransformComponent>(e, t_comp);
};

void
remove_renderable(entt::registry& r, const entt::entity& e)
{
  r.remove<SpriteComponent>(e);
  r.remove<TransformComponent>(e);
};

void
set_parent(entt::registry& r, const entt::entity& e, const entt::entity& parent)
{
  r.emplace<EntityHierarchyComponent>(e, parent);
}

void
add_child(entt::registry& r, const entt::entity& e, const entt::entity& child)
{
  auto& hc = r.get<EntityHierarchyComponent>(e);
  hc.children.push_back(child);
}

entt::entity
create_entity(entt::registry& r, const ENTITY_TYPE& type)
{
  const auto& h = r.ctx().at<SINGLETON_HierarchyComponent>();
  auto e = r.create();
  add_child(r, h.root_node, e);
  set_parent(r, e, h.root_node);
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
      create_renderable(r, e, type);
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

      auto line_u = r.create();
      r.emplace<TagComponent>(line_u, "line-u");
      create_renderable(r, line_u, ENTITY_TYPE::FREE_CURSOR);
      add_child(r, h.root_node, line_u);
      set_parent(r, line_u, h.root_node);

      auto line_d = r.create();
      r.emplace<TagComponent>(line_d, "line-d");
      create_renderable(r, line_d, ENTITY_TYPE::FREE_CURSOR);
      add_child(r, h.root_node, line_d);
      set_parent(r, line_d, h.root_node);

      auto line_l = r.create();
      r.emplace<TagComponent>(line_l, "line-l");
      create_renderable(r, line_l, ENTITY_TYPE::FREE_CURSOR);
      add_child(r, h.root_node, line_l);
      set_parent(r, line_l, h.root_node);

      auto line_r = r.create();
      r.emplace<TagComponent>(line_r, "line-r");
      create_renderable(r, line_r, ENTITY_TYPE::FREE_CURSOR);
      add_child(r, h.root_node, line_r);
      set_parent(r, line_r, h.root_node);

      auto backdrop = r.create();
      r.emplace<TagComponent>(backdrop, "backdrop");
      create_renderable(r, backdrop, ENTITY_TYPE::FREE_CURSOR, colours.backdrop_red);
      add_child(r, h.root_node, backdrop);
      set_parent(r, backdrop, h.root_node);

      // object tag
      FreeCursorComponent c;
      c.line_u = line_u;
      c.line_d = line_d;
      c.line_l = line_l;
      c.line_r = line_r;
      c.backdrop = backdrop;
      r.emplace<FreeCursorComponent>(e, c);
      // physics
      r.emplace<PhysicsActorComponent>(e, GameCollisionLayer::ACTOR_CURSOR);
      r.emplace<PhysicsSizeComponent>(e, PhysicsSizeComponent(SPRITE_SIZE, SPRITE_SIZE));
      break;
    }

    case ENTITY_TYPE::GRID_CURSOR: {

      auto line_u = r.create();
      r.emplace<TagComponent>(line_u, "line-u");
      create_renderable(r, line_u, ENTITY_TYPE::GRID_CURSOR, colours.cyan);
      add_child(r, h.root_node, line_u);
      set_parent(r, line_u, h.root_node);

      auto line_d = r.create();
      r.emplace<TagComponent>(line_d, "line-d");
      create_renderable(r, line_d, ENTITY_TYPE::GRID_CURSOR, colours.cyan);
      add_child(r, h.root_node, line_d);
      set_parent(r, line_d, h.root_node);

      auto line_l = r.create();
      r.emplace<TagComponent>(line_l, "line-l");
      create_renderable(r, line_l, ENTITY_TYPE::GRID_CURSOR, colours.cyan);
      add_child(r, h.root_node, line_l);
      set_parent(r, line_l, h.root_node);

      auto line_r = r.create();
      r.emplace<TagComponent>(line_r, "line-r");
      create_renderable(r, line_r, ENTITY_TYPE::GRID_CURSOR, colours.cyan);
      add_child(r, h.root_node, line_r);
      set_parent(r, line_r, h.root_node);

      auto backdrop = r.create();
      r.emplace<TagComponent>(backdrop, "backdrop");
      create_renderable(r, backdrop, ENTITY_TYPE::GRID_CURSOR, colours.dblue);
      add_child(r, h.root_node, backdrop);
      set_parent(r, backdrop, h.root_node);

      GridCursorComponent c;
      c.line_u = line_u;
      c.line_d = line_d;
      c.line_l = line_l;
      c.line_r = line_r;
      c.backdrop = backdrop;
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