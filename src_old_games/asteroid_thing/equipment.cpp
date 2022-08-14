// #include "equipment.hpp"

// PhysicsSizeComponent
// create_turret_physics_size_component(entt::registry& r)
// {
//   PhysicsSizeComponent comp;
//   comp.h = SPRITE_SIZE;
//   comp.w = SPRITE_SIZE;
//   return comp;
// }

// SpriteComponent
// create_turret_sprite_component(entt::registry& r)
// {
//   const auto& slots = r.ctx().at<SINGLETON_Textures>();
//   const auto& colours = r.ctx().at<SINGLETON_ColoursComponent>();
//   const auto& sprites = r.ctx().at<SINGLETON_Animations>();

//   SpriteComponent comp;
//   comp.colour = engine::SRGBToLinear(colours.turret);
//   comp.tex_unit = slots.tex_unit_kenny;

//   // search kenny-nl spritesheet
//   const auto anim = find_animation(sprites.animations, "PERSON_1");
//   comp.x = anim.animation_frames[0].x;
//   comp.y = anim.animation_frames[0].y;
//   comp.angle_radians = anim.animation_angle_degrees * engine::PI / 180.0f;

//   return comp;
// }

// TransformComponent
// create_turret_transform_component(entt::registry& r)
// {
//   TransformComponent comp;
//   comp.scale.x = SPRITE_SIZE;
//   comp.scale.y = SPRITE_SIZE;
//   return comp;
// }

// entt::entity
// create_turret(entt::registry& r)
// {
//   const auto& h = r.ctx().at<SINGLETON_HierarchyComponent>();
//   auto& hc = r.get<EntityHierarchyComponent>(h.root_node);
//   auto e = r.create();
//   hc.children.push_back(e);

//   r.emplace<TagComponent>(e, "turret");
//   r.emplace<EntityHierarchyComponent>(e, h.root_node);
//   r.emplace<SpriteComponent>(e, create_turret_sprite_component(r));
//   r.emplace<TransformComponent>(e, create_turret_transform_component(r));
//   r.emplace<PhysicsActorComponent>(e, GameCollisionLayer::ACTOR_PLAYER_PET);
//   r.emplace<PhysicsSizeComponent>(e, create_turret_physics_size_component(r));
//   r.emplace<VelocityComponent>(e);
//   // gameplay
//   r.emplace<TurretComponent>(e);
//   r.emplace<BreakableComponent>(e, 100);
//   return e;
// }

// entt::entity
// create_potion(entt::registry& r)
// {
//   const auto& h = r.ctx().at<SINGLETON_HierarchyComponent>();
//   auto& hc = r.get<EntityHierarchyComponent>(h.root_node);
//   auto e = r.create();
//   hc.children.push_back(e);

//   r.emplace<TagComponent>(e, "potion");
//   r.emplace<Potion>(e);
// }