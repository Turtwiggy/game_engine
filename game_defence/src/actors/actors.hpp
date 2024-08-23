#pragma once

#include "actors/base.hpp"
#include "colour/colour.hpp"
#include "modules/combat_damage/components.hpp"

#include <entt/entt.hpp>

// #include <concepts>

namespace game2d {

//
// Basically constructors for the entities
// i.e. data that needs to be set per entity
//

struct ActorBreachCharge : public EntityDescription
{
  bool placeholder : true;
};

struct ActorDungeon : public EntityDescription
{
  AvailableTeams team = AvailableTeams::neutral;
  int hp = 100;
  int max_hp = 100;
  engine::SRGBColour hovered_colour;
};

struct ActorJetpackPlayer : public EntityDescription
{
  AvailableTeams team = AvailableTeams::neutral;
};

struct ActorSpaceShip : public EntityDescription
{
  AvailableTeams team = AvailableTeams::neutral;
};

struct ActorSpaceCapsule : public EntityDescription
{
  bool placeholder = true;
};

struct ActorSpaceCargo : public EntityDescription
{
  bool placeholder = true;
};

struct BulletBouncy : public EntityDescription
{
  int bullet_damage = 12;
  float bullet_speed = 250;
  float rotation = 0.0f;
  glm::vec2 dir{ 0.0f };
  AvailableTeams team = AvailableTeams::neutral;
  entt::entity parent = entt::null;
};

struct BulletDefault : public EntityDescription
{
  int bullet_damage = 12;
  float bullet_speed = 250.0f;
  float rotation = 0.0f;
  glm::vec2 dir{ 0.0f };
  AvailableTeams team = AvailableTeams::neutral;
  entt::entity parent = entt::null;
};

struct Particle : public EntityDescription
{
  int time_to_live_ms = 3 * 1000;
  float start_size = 10;
  float end_size = 10;
  glm::vec2 velocity{ 0, 0 };
  // int amount = 1;
  // bool slightly_change_velocity = false;
  // not implemented
  // engine::SRGBColour start_colour;
  // engine::SRGBColour end_colour;
};

struct SolidWall : public EntityDescription
{
  bool placeholder = true;
};

struct WeaponShotgun : public EntityDescription
{
  entt::entity parent = entt::null;
  bool able_to_shoot = false;
  AvailableTeams team = AvailableTeams::neutral;
};

//
// Factories
//

// #define REGISTER_DERIVED_TYPE(entity_type, entity_description_struct)                                                       \
//   template<>                                                                                                                \
//   struct DerivedTypeMap<entity_type>                                                                                        \
//   {                                                                                                                         \
//     using type = entity_description_struct;                                                                                 \
//   };

// template<EntityType>
// struct DerivedTypeMap;

#define GENERATE_FACTORY(entity_type, entity_description_struct)                                                            \
  struct Factory_##entity_description_struct : public Actor<entity_type>                                                    \
  {                                                                                                                         \
    Factory_##entity_description_struct() = delete;                                                                         \
    static entt::entity create(entt::registry& r, const entity_description_struct& desc);                                   \
  };                                                                                                                        \
  // REGISTER_DERIVED_TYPE(entity_type, Factory_##entity_description_struct)

GENERATE_FACTORY(EntityType::actor_breach_charge, ActorBreachCharge)
GENERATE_FACTORY(EntityType::actor_dungeon, ActorDungeon)
GENERATE_FACTORY(EntityType::actor_jetpack_player, ActorJetpackPlayer)
GENERATE_FACTORY(EntityType::actor_space_ship, ActorSpaceShip)
GENERATE_FACTORY(EntityType::actor_space_cargo, ActorSpaceCargo)
GENERATE_FACTORY(EntityType::actor_space_capsule, ActorSpaceCapsule)
GENERATE_FACTORY(EntityType::bullet_bouncy, BulletBouncy)
GENERATE_FACTORY(EntityType::bullet_default, BulletDefault)
GENERATE_FACTORY(EntityType::particle, Particle)
// GENERATE_FACTORY(EntityType::particle_emitter, Particle)
GENERATE_FACTORY(EntityType::solid_wall, SolidWall)
GENERATE_FACTORY(EntityType::weapon_shotgun, WeaponShotgun)
// GENERATE_FACTORY(EntityType::quip, WeaponShotgun)

// template<EntityType E>
// using DerivedType = typename DerivedTypeMap<E>::type;

//
// Provide a static concept check that all enum values have derived classes
//

/*

template<typename T, EntityType ET>
concept enum_has_factory = std::is_base_of_v<Actor<ET>, T>;

*/

/*

template<std::size_t... Is>
constexpr bool
check_all_enum_values(std::index_sequence<Is...>)
{
  return (enum_has_factory<DerivedType<static_cast<EntityType>(Is)>, static_cast<EntityType>(Is)> && ...);
};
constexpr bool
validate_all_enum_classes()
{
  return check_all_enum_values(std::make_index_sequence<static_cast<std::size_t>(EntityType::count)>{});
}
*/

// static_assert(validate_all_enum_classes(), "Not all enum values have corresponding derived classes!");

// #undef REGISTER_DERIVED_TYPE
#undef GENERATE_FACTORY

//
// TEMPORARY
// it's likely that everything that uses create_transform
// should be it's own EntityType(?)
//
entt::entity
create_transform(entt::registry& r);

void
add_components(entt::registry& r, const entt::entity e, const ActorDungeon& desc);
void
remove_components(entt::registry& r, const entt::entity e, const ActorJetpackPlayer& desc);

} // namespace game2d