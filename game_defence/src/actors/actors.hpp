#pragma once

#include "actors/bags/core.hpp"
#include "bags/armour.hpp"
#include "bags/bullets.hpp"
#include "bags/items.hpp"
#include "bags/weapons.hpp"
#include "modules/combat_damage/components.hpp"
#include "modules/ui_inventory/components.hpp"

#include <entt/entt.hpp>

namespace game2d {

//
// Basically constructors for the entities
// i.e. data that needs to be set per entity
//

struct DataDungeonActor : public EntityData
{
  AvailableTeams team = AvailableTeams::neutral;
  int hp = 100;
  int max_hp = 100;
};

struct DataDungeonCover : public EntityData
{
  bool placeholder = true;

  DataDungeonCover() { sprite = "WALL_19_1_NONE"; }
};

struct DataDungeonLootbag : public EntityData
{
  DefaultInventory inventory;

  DataDungeonLootbag() = delete;
  DataDungeonLootbag(const DefaultInventory inv)
    : inventory(inv)
  {
    sprite = "SKULL_AND_BONES";
  };
};

struct DataJetpackActor : public EntityData
{
  AvailableTeams team = AvailableTeams::neutral;

  DataJetpackActor() { sprite = "PERSON_25_0"; }
};

struct DataSpaceShipActor : public EntityData
{
  AvailableTeams team = AvailableTeams::neutral;

  DataSpaceShipActor() { sprite = "SPACE_VEHICLE_1"; }
};

struct DataSpaceCapsuleActor : public EntityData
{
  bool placeholder = true;
};

struct DataSpaceCargoActor : public EntityData
{
  DataSpaceCargoActor()
  {
    sprite = "DICE_DARK_X";
    // colour = get_srgb_colour_by_tag("");
  }
};

struct DataParticle : public EntityData
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

struct DataParticleEmitter : public EntityData
{
  entt::entity parent = entt::null;
  glm::vec2 velocity{ 0.0f, 0.0f };

  float start_size = 6.0f;
  float end_size = 2.0f;
};

struct DataSolidWall : public EntityData
{
  bool placeholder = true;
};

//
// Factories
//

#define GENERATE_FACTORY(entity_description_struct)                                                                         \
  struct Factory_##entity_description_struct                                                                                \
  {                                                                                                                         \
    static entt::entity create(entt::registry& r, const entity_description_struct& desc);                                   \
  };

GENERATE_FACTORY(DataDungeonActor)
GENERATE_FACTORY(DataDungeonCover)
GENERATE_FACTORY(DataDungeonLootbag)
GENERATE_FACTORY(DataJetpackActor)
GENERATE_FACTORY(DataSpaceShipActor)
GENERATE_FACTORY(DataSpaceCapsuleActor)
GENERATE_FACTORY(DataSpaceCargoActor)
GENERATE_FACTORY(DataParticle)
GENERATE_FACTORY(DataParticleEmitter)
GENERATE_FACTORY(DataSolidWall)
// GENERATE_FACTORY(EntityType::quip, WeaponShotgunDesc)

// hmm
GENERATE_FACTORY(DataArmour)
GENERATE_FACTORY(DataBullet)
GENERATE_FACTORY(DataWeaponShotgun)
// items
GENERATE_FACTORY(DataBreachCharge)
GENERATE_FACTORY(DataMedkit)
GENERATE_FACTORY(DataScrap)

#undef GENERATE_FACTORY

//
// it's likely that everything that uses create_transform
// should be it's own EntityType(?)
//
entt::entity
create_transform(entt::registry& r);

// takes in desc so can be overloaded
void
add_components(entt::registry& r, const entt::entity e, const DataDungeonActor& desc);
void
add_components(entt::registry& r, const entt::entity e, const DataWeaponShotgun& desc);
void
add_components(entt::registry& r, const entt::entity e, const DataBreachCharge& desc);
void
add_components(entt::registry& r, const entt::entity e, const DataSolidWall& desc);

// takes in desc so can be overloaded
void
remove_components(entt::registry& r, const entt::entity e, const DataJetpackActor& desc);

} // namespace game2d