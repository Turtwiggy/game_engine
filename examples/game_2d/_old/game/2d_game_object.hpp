#pragma once

// c++ lib headers
#include <map>

// other project headers
#include <glm/glm.hpp>

// your includes
#include "constants.hpp"
#include "engine/maths.hpp"
#include "spritemap.hpp"

namespace game2d {

enum class AiBehaviour
{
  MOVEMENT_DIRECT,
  MOVEMENT_ARC_ANGLE,
};

enum class EditorMode
{
  EDITOR_PLACE_MODE,
  EDITOR_SELECT_MODE,
  PLAYER_ATTACK,
};

enum class GameRunning
{
  ACTIVE,
  PAUSED,
  GAME_OVER
};

enum class GamePhase
{
  ATTACK,
  SHOP,
};

enum class CollisionLayer
{
  NoCollision = 0,
  Bullet = 1,
  Player = 2,
  Enemy = 3,
  Obstacle = 4,
  Weapon = 5,

  Count = 6
};

// A "PlaceableEntity" can be placed on the grid
enum class PlaceableEntity
{
  TREE = 0,
  // SHOP = 1,
};

//
// If this game ever has more than 1 million entities,
// reconsider this structure. Until then, lets gooooo.
//
struct GameObject2D
{
private:
  static inline uint32_t global_int_counter = 0;

public:
  uint32_t id = 0;

  // render
  int tex_slot = 0;
  sprite::type sprite = sprite::type::SQUARE;

  float angle_radians = 0.0f;
  glm::vec4 colour = { 1.0f, 0.0f, 0.0f, 1.0f };
  glm::ivec2 render_size = { PIXELS_TO_RENDER, PIXELS_TO_RENDER };

  // physics
  glm::ivec2 physics_size = render_size;
  CollisionLayer collision_layer = CollisionLayer::NoCollision;
  std::vector<glm::ivec2> in_physics_grid_cell;

  // game: movement
  float speed_current = 50.0f;
  float speed_default = 50.0f;
  glm::vec2 velocity = { 0.0f, 0.0f };
  float velocity_boost_modifier = 5.0f;
  float shift_boost_time = 10.0f;
  float shift_boost_time_left = shift_boost_time;

  // ai priority list. higher priority later in list.
  std::vector<AiBehaviour> ai_priority_list;
  float approach_theta_degrees = 0.0f;
  int damage_to_give_player = 0;

  // game: inventory
  int equipped_item_index = 0;

  // game: shooting
  float bullet_seconds_between_spawning = 0.15f;
  float bullet_seconds_between_spawning_left = 0.0f;
  int bullets_in_a_clip = 5;
  int bullets_in_a_clip_left = bullets_in_a_clip;
  float reload_time = 0.3f;
  float reload_time_left = reload_time;

  // game: lifecycle timed
  float time_alive = 5.0f;
  float time_alive_left = 5.0f;

  // game: lifecycle health
  int damage_able_to_be_taken = 6;
  int damage_taken = 0;
  bool invulnerable = false;
  std::vector<int> attack_ids_taken_damage_from;

  // vfx
  float flash_time_left = 0.0f;
  glm::vec4 flash_colour = { 1.0f, 1.0f, 1.0f, 1.0f };
  glm::vec4 original_colour = colour;

  // game: extra
  std::string name = "DEFAULT";

  GameObject2D() { id = ++GameObject2D::global_int_counter; }
};

// util

[[nodiscard]] glm::ivec2
gameobject_in_worldspace(const GameObject2D& camera, const GameObject2D& go);

[[nodiscard]] glm::ivec2
convert_top_left_to_centre(const GameObject2D& go);

[[nodiscard]] bool
gameobject_off_screen(glm::vec2 pos, glm::vec2 size, const glm::ivec2& screen_size);

namespace gameobject {

// logic

void
update_entities_lifecycle(std::vector<GameObject2D>& objs, const float delta_time_s);

void
erase_entities_that_are_flagged_for_delete(std::vector<GameObject2D>& objs, const float delta_time_s);

// entities

GameObject2D
create_generic();

// specific entities

GameObject2D
create_bullet(sprite::type sprite, glm::vec4 colour);

GameObject2D
create_enemy(engine::RandomState& rnd);

GameObject2D
create_light();

GameObject2D
create_tree();

GameObject2D
create_player(sprite::type sprite, int tex_slot, glm::vec4 colour, glm::vec2 screen);

GameObject2D
create_weapon(sprite::type sprite, int tex_slot, glm::vec4 colour);

} // namespace gameobject

} // namespace game2d
