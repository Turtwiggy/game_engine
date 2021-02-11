
// header
#include "breakout/game.hpp"

// C++ lib headers
#include <fstream>
#include <iostream>
#include <sstream>

// other project headers
#include "thirdparty/magic_enum.hpp"

namespace game2d {

void
reset_ball(Ball& ball)
{}

void
move_ball(Ball& ball, float delta_time_s, int window_width)
{
  if (!ball.stuck) {
    Transform& transform = ball.game_object.transform;
    transform.position += ball.game_object.velocity * delta_time_s;

    // if ball goes off left of screen...
    if (transform.position.x <= 0.0f) {
      ball.game_object.velocity.x = glm::abs(ball.game_object.velocity.x);
      transform.position.x = 0.0f;
    }
    // if ball goes off right of screen...
    else if (transform.position.x + transform.scale.x >= window_width) {
      ball.game_object.velocity.x = -glm::abs(ball.game_object.velocity.x);
      transform.position.x = window_width - transform.scale.x;
    }
    // if ball goes off top of screen...
    if (transform.position.y <= 0.0f) {
      ball.game_object.velocity.y = glm::abs(ball.game_object.velocity.y);
      transform.position.y = 0.0f;
    }
  }
}

// ---- level load functions

void
load_level_from_file(std::vector<std::vector<int>>& layout, const std::string& path)
{
  std::ifstream fstream(path);

  if (fstream) {
    int tile_code;
    std::string line;
    while (std::getline(fstream, line)) {
      // std::cout << line << "\n";

      std::istringstream sstream(line);
      std::vector<int> row;

      while (sstream >> tile_code) {
        // std::cout << tile_code << "\n";

        row.push_back(tile_code);
      }

      if (row.size() > 0)
        layout.push_back(row);
    }
  } else {
    std::cout << "error reading game level file! \n";
  }
};

void
init_level(GameLevel& level, const std::vector<std::vector<int>>& layout, int level_width, int level_height)
{
  level.bricks.clear();

  int height = static_cast<int>(layout.size());
  int width = static_cast<int>(layout[0].size());
  // printf("level, h: %i w: %i \n", height, width);

  float unit_width = level_width / static_cast<float>(width);
  float unit_height = level_height / height;

  for (int y = 0; y < height; ++y) {
    for (int x = 0; x < width; ++x) {
      int tile_data = layout[y][x];

      glm::vec2 pos(unit_width * x, unit_height * y);
      glm::vec2 size(unit_width, unit_height);
      // printf("tile_data: %i pos x: %f y: %f size x: %f y: %f \n", tile_data, pos.x, pos.y, size.x, size.y);

      if (tile_data == 1) {
        // GameObject go{ fightingengine::ResourceManager::get_texture("block_solid") };

        // // transform
        // go.transform.position = pos;
        // go.transform.scale = size;
        // go.transform.colour = { 0.8f, 0.8f, 0.7f };
        // go.transform.angle = 0.0f;
        // // state
        // go.is_solid = true;

        // level.bricks.push_back(go);
      } else {
        // GameObject go{ fightingengine::ResourceManager::get_texture("block") };

        // glm::vec3 color = glm::vec3(1.0f); // original: white
        // if (tile_data == 2)
        //   color = glm::vec3(0.2f, 0.6f, 1.0f); // blueish
        // else if (tile_data == 3)
        //   color = glm::vec3(0.0f, 0.7f, 0.0f); // greenish
        // else if (tile_data == 4)
        //   color = glm::vec3(0.8f, 0.8f, 0.4f);
        // else if (tile_data == 5)
        //   color = glm::vec3(1.0f, 0.5f, 0.0f);

        // // transform
        // go.transform.position = pos;
        // go.transform.scale = size;
        // go.transform.colour = color;
        // go.transform.angle = 0.0f;
        // // state
        // go.is_solid = false;

        // level.bricks.push_back(go);
      }
    }
  }
}

// ---- simple aabb collisions

// void do_collisions_bricks( GameLevel& objects, Ball& ball )
// {
//     for ( auto& box : objects.bricks )
//     {
//         if ( ! box.destroyed )
//         {
//             CollisionInfo collision = has_collided( ball, box );

//             if ( ! std::get<0>(collision) ) // no collision
//                 continue;

//             if( ! box.is_solid )
//                 box.destroyed = true;

//             // collision resolution
//             CollisionDirection dir = std::get<1>(collision);
//             glm::vec2 difference = std::get<2>(collision);

//             printf("collision: %s \n", std::string(magic_enum::enum_name(dir)).c_str());

//             //horizontal collisions

//             if ( dir == CollisionDirection::COLLISION_LEFT )
//             {
//                 ball.game_object.velocity.x = -glm::abs(ball.game_object.velocity.x);

//                 float depth = ball.radius - glm::abs( difference.x );
//                 ball.game_object.transform.position.x += depth; //move ball right
//             }

//             else if ( dir == CollisionDirection::COLLISION_RIGHT )
//             {
//                 ball.game_object.velocity.x = glm::abs(ball.game_object.velocity.x);

//                 float depth = ball.radius - glm::abs( difference.x );
//                 ball.game_object.transform.position.x -= depth;  //move ball left
//             }

//             //vertical collisions

//             else if ( dir == CollisionDirection::COLLISION_UP )
//             {
//                 ball.game_object.velocity.y = -glm::abs(ball.game_object.velocity.y);

//                 float depth = ball.radius - glm::abs( difference.y );
//                 ball.game_object.transform.position.y -= depth; //move ball up
//             }

//             else if ( dir == CollisionDirection::COLLISION_DOWN )
//             {
//                 ball.game_object.velocity.y = glm::abs(ball.game_object.velocity.y);

//                 float depth = ball.radius - glm::abs( difference.y );
//                 ball.game_object.transform.position.y += depth; //move ball down
//             }

//         }
//     }
// }

// void do_collisions_player( GameObject& player, Ball& ball )
// {

//     if( ! ball.stuck && std::get<0>(info) )
//     {
//        // printf("ball collided with paddle!");

//         float center = player.transform.position.x + player.transform.scale.x / 2.0f;
//         float distance = ball.game_object.transform.position.x + ball.radius - center;
//         float percentage = distance / player.transform.scale.x / 2.0f;
//         //printf("percentage: %f", percentage);

//         float strength = 2.0f;
//         glm::vec2 old_velocity = ball.game_object.velocity;
//         float initial_ball_velocity = 100.0f;
//         ball.game_object.velocity.x = initial_ball_velocity * percentage * strength;
//         ball.game_object.velocity.y = -glm::abs(ball.game_object.velocity.y);
//         ball.game_object.velocity = glm::normalize( ball.game_object.velocity ) * glm::length(old_velocity);
//     }
// }

// ---- breakout game functions

void
update_user_input(Application& app, float delta_time_s, GameObject& player, Ball& ball, float screen_width)
{
  if (app.get_input().get_key_held(SDL_SCANCODE_A)) {
    float velocity_x = player.velocity.x * delta_time_s;
    if (player.transform.position.x >= 0.0f) {
      player.transform.position.x -= velocity_x;

      if (ball.stuck)
        ball.game_object.transform.position.x -= velocity_x;
    }
  }
  if (app.get_input().get_key_held(SDL_SCANCODE_D)) {
    float velocity_x = player.velocity.x * delta_time_s;
    if (player.transform.position.x <= screen_width - player.transform.scale.x) {
      player.transform.position.x += velocity_x;

      if (ball.stuck)
        ball.game_object.transform.position.x += velocity_x;
    }
  }
  if (app.get_input().get_key_down(SDL_KeyCode::SDLK_SPACE)) {
    printf("ball unstuck \n");
    ball.stuck = false;
  }
}

void
update_game_state()
{
  //
}

} // namespace game2d
