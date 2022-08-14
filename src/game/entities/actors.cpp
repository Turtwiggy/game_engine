#include "actors.hpp"

#include "engine/maths/maths.hpp"
#include "game/components/hp.hpp"
#include "modules/lifecycle/components.hpp"
#include "modules/physics/components.hpp"
#include "modules/renderer/components.hpp"
#include "modules/sprites/components.hpp"
#include "modules/sprites/helpers.hpp"
#include "modules/ui_hierarchy/components.hpp"
#include "resources/colour.hpp"
#include "resources/textures.hpp"

namespace game2d {

static constexpr int SPRITE_SIZE = 16 * 2;

PhysicsSizeComponent
create_player_physics_size_component(entt::registry& r)
{
  PhysicsSizeComponent comp;
  comp.h = SPRITE_SIZE;
  comp.w = SPRITE_SIZE;
  return comp;
}

SpriteComponent
create_player_sprite_component(entt::registry& r)
{
  const auto& slots = r.ctx().at<SINGLETON_Textures>();
  const auto& colours = r.ctx().at<SINGLETON_ColoursComponent>();
  const auto& sprites = r.ctx().at<SINGLETON_Animations>();

  SpriteComponent comp;
  comp.colour = engine::SRGBToLinear(colours.player_unit);
  comp.tex_unit = slots.tex_unit_kenny;

  // search kenny-nl spritesheet
  const auto anim = find_animation(sprites.animations, "EMPTY");
  comp.x = anim.animation_frames[0].x;
  comp.y = anim.animation_frames[0].y;

  return comp;
}

TransformComponent
create_player_transform_component(entt::registry& r)
{
  TransformComponent comp;
  comp.scale.x = SPRITE_SIZE;
  comp.scale.y = SPRITE_SIZE;
  return comp;
}

entt::entity
create_player(entt::registry& r)
{
  const auto& h = r.ctx().at<SINGLETON_HierarchyComponent>();
  auto& hc = r.get<EntityHierarchyComponent>(h.root_node);

  auto e = r.create();
  hc.children.push_back(e);

  r.emplace<TagComponent>(e, "player");
  r.emplace<EntityHierarchyComponent>(e, h.root_node);
  r.emplace<SpriteComponent>(e, create_player_sprite_component(r));
  r.emplace<TransformComponent>(e, create_player_transform_component(r));
  r.emplace<PhysicsActorComponent>(e, GameCollisionLayer::ACTOR_PLAYER);
  r.emplace<PhysicsSizeComponent>(e, create_player_physics_size_component(r));
  r.emplace<VelocityComponent>(e);
  // gameplay
  r.emplace<PlayerComponent>(e);
  r.emplace<HpComponent>(e, 100);
  return e;
}

//

PhysicsSizeComponent
create_asteroid_physics_size_component(entt::registry& r)
{
  PhysicsSizeComponent comp;
  comp.h = SPRITE_SIZE;
  comp.w = SPRITE_SIZE;
  return comp;
}

SpriteComponent
create_asteroid_sprite_component(entt::registry& r)
{
  const auto& slots = r.ctx().at<SINGLETON_Textures>();
  const auto& colours = r.ctx().at<SINGLETON_ColoursComponent>();
  const auto& sprites = r.ctx().at<SINGLETON_Animations>();

  SpriteComponent comp;
  comp.colour = engine::SRGBToLinear(colours.asteroid);
  comp.tex_unit = slots.tex_unit_kenny;

  // search kenny-nl spritesheet
  const auto anim = find_animation(sprites.animations, "EMPTY");
  comp.x = anim.animation_frames[0].x;
  comp.y = anim.animation_frames[0].y;

  return comp;
}

TransformComponent
create_asteroid_transform_component(entt::registry& r)
{
  TransformComponent comp;
  comp.scale.x = SPRITE_SIZE;
  comp.scale.y = SPRITE_SIZE;

  comp.position.x = 100.0f; // temp
  comp.position.y = 100.0f; // temp

  return comp;
}

entt::entity
create_asteroid(entt::registry& r)
{
  const auto& h = r.ctx().at<SINGLETON_HierarchyComponent>();
  auto& hc = r.get<EntityHierarchyComponent>(h.root_node);

  auto e = r.create();
  hc.children.push_back(e);
  r.emplace<AsteroidComponent>(e);

  r.emplace<TagComponent>(e, "asteroid-duck");
  r.emplace<EntityHierarchyComponent>(e, h.root_node);
  r.emplace<SpriteComponent>(e, create_asteroid_sprite_component(r));
  r.emplace<TransformComponent>(e, create_asteroid_transform_component(r));
  r.emplace<PhysicsActorComponent>(e, GameCollisionLayer::ACTOR_ASTEROID);
  r.emplace<PhysicsSizeComponent>(e, create_asteroid_physics_size_component(r));
  r.emplace<VelocityComponent>(e);
  r.emplace<EntityTimedLifecycle>(e);
  return e;
}

//

PhysicsSizeComponent
create_bullet_physics_size_component(entt::registry& r)
{
  PhysicsSizeComponent comp;
  comp.h = SPRITE_SIZE / 2;
  comp.w = SPRITE_SIZE / 2;
  return comp;
}

SpriteComponent
create_bullet_sprite_component(entt::registry& r)
{
  const auto& slots = r.ctx().at<SINGLETON_Textures>();
  const auto& colours = r.ctx().at<SINGLETON_ColoursComponent>();
  const auto& sprites = r.ctx().at<SINGLETON_Animations>();

  SpriteComponent comp;
  comp.colour = engine::SRGBToLinear(colours.bullet);
  comp.tex_unit = slots.tex_unit_kenny;

  // search kenny-nl spritesheet
  const auto anim = find_animation(sprites.animations, "PERSON_1");
  comp.x = anim.animation_frames[0].x;
  comp.y = anim.animation_frames[0].y;
  comp.angle_radians = anim.animation_angle_degrees * engine::PI / 180.0f;

  return comp;
}

TransformComponent
create_bullet_transform_component(entt::registry& r)
{
  TransformComponent comp;
  comp.scale.x = SPRITE_SIZE / 2;
  comp.scale.y = SPRITE_SIZE / 2;
  return comp;
}

entt::entity
create_bullet(entt::registry& r)
{
  const auto& h = r.ctx().at<SINGLETON_HierarchyComponent>();
  auto& hc = r.get<EntityHierarchyComponent>(h.root_node);

  auto e = r.create();
  hc.children.push_back(e);

  r.emplace<TagComponent>(e, "bullet");
  r.emplace<EntityHierarchyComponent>(e, h.root_node);
  r.emplace<SpriteComponent>(e, create_bullet_sprite_component(r));
  r.emplace<TransformComponent>(e, create_bullet_transform_component(r));
  r.emplace<PhysicsActorComponent>(e, GameCollisionLayer::ACTOR_BULLET);
  r.emplace<PhysicsSizeComponent>(e, create_bullet_physics_size_component(r));
  r.emplace<VelocityComponent>(e);
  r.emplace<EntityTimedLifecycle>(e, 20000); // bullet time alive
  return e;
}

}