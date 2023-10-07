#pragma once

#include "actors.hpp"

#include "colour/colour.hpp"

#include <thread>

namespace game2d {

struct SpawnerComponent
{
  EntityType type_to_spawn = EntityType::actor_enemy;

  bool continuous_spawn = true;

  // default class
  bool class_set = false;
  int hp = 1;
  int attack = 1;
  std::shared_ptr<engine::LinearColour> colour = nullptr;
  std::string sprite = "PERSON_25_1";
};

} // namespace game2d