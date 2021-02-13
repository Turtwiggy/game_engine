#pragma once

// c++ standard lib headers
#include <tuple>
#include <vector>

// other project headers
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

// your project headers
#include "engine/application.hpp"
#include "engine/opengl/render_command.hpp"
#include "engine/opengl/texture.hpp"
using namespace fightingengine;

namespace game2d {

enum class GameState
{
  GAME_ACTIVE,
  GAME_MENU,
  GAME_WIN
};

struct Transform
{
  glm::vec2 position = { 0.0f, 0.0f }; // in pixels, centered
  float angle = 0.0f;                  // in degrees
  glm::vec2 scale = { 100.0f, 100.0f };
  glm::vec4 colour = { 1.0f, 1.0f, 1.0f, 1.0f };
};

struct GameObject
{
  Transform transform;
  glm::vec2 velocity = { 0.0f, 0.0f };
  int tex_slot = 0;

  // gameobject flags...
  // note: avoid adding these, or come up with a better system
  // more flags is a 2^n of configurations of testing to make sure everything
  bool is_solid = false;
  bool destroyed = false;

  GameObject() = default;
};

struct Ball
{
  GameObject game_object;

  float radius = 1.0f;
  bool stuck = true;

  Ball() = default;
};

void
reset_ball(Ball& ball);

void
move_ball(Ball& ball, float delta_time_s, int window_width);

//
// GameLevel
//

struct GameLevel
{
  std::vector<GameObject> bricks;
};

void
load_level_from_file(std::vector<std::vector<int>>& layout, const std::string& path);

void
init_level(GameLevel& level, const std::vector<std::vector<int>>& layout, int width, int height);

struct Breakout
{
  GameState state = GameState::GAME_ACTIVE;

  std::vector<GameLevel> levels;
};

// ---- simple aabb collisions

// enum class CollisionDirection
// {
//     COLLISION_LEFT,
//     COLLISION_RIGHT,
//     COLLISION_UP,
//     COLLISION_DOWN,

//     COLLISION_NONE
// };
// typedef std::tuple<bool, CollisionDirection, glm::vec2> CollisionInfo;

// void do_collisions_bricks( GameLevel& objects, Ball& ball );
// void do_collisions_player( GameObject& player, Ball& ball );

} // namespace game2d