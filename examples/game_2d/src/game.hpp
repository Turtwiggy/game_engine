// #pragma once

// // c++ standard lib headers
// #include <tuple>
// #include <vector>

// // other project headers
// #include <glm/glm.hpp>
// #include <glm/gtc/matrix_transform.hpp>

// // your project headers
// #include "engine/application.hpp"
// #include "engine/opengl/render_command.hpp"
// #include "engine/opengl/texture.hpp"
// using namespace fightingengine;

// namespace game2d {

// //
// // GameLevel
// //

// struct GameLevel
// {
//   std::vector<GameObject> bricks;
// };

// void
// load_level_from_file(std::vector<std::vector<int>>& layout, const std::string& path);

// void
// init_level(GameLevel& level, const std::vector<std::vector<int>>& layout, int width, int height);

// struct Breakout
// {
//   GameState state = GameState::GAME_ACTIVE;

//   std::vector<GameLevel> levels;
// };

// // ---- simple aabb collisions

// // enum class CollisionDirection
// // {
// //     COLLISION_LEFT,
// //     COLLISION_RIGHT,
// //     COLLISION_UP,
// //     COLLISION_DOWN,

// //     COLLISION_NONE
// // };
// // typedef std::tuple<bool, CollisionDirection, glm::vec2> CollisionInfo;

// // void do_collisions_bricks( GameLevel& objects, Ball& ball );
// // void do_collisions_player( GameObject& player, Ball& ball );

// } // namespace game2d