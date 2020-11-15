#pragma once

//other project headers
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

//your project headers
#include "engine/resources/resource_manager.hpp"
#include "engine/graphics/shader.hpp"
#include "engine/graphics/texture.hpp"

namespace game2d 
{

enum class BreakoutGameState 
{
   GAME_ACTIVE,
   GAME_MENU,
   GAME_WIN
};

struct BreakoutData
{
    fightingengine::Shader sprite_shader;
    fightingengine::Texture2D sprite_texture;

    glm::mat4 projection; //this should probably be in a camera2d class

    BreakoutGameState state;
};

void init_breakout(BreakoutData& b, const int& width, const int& height);



//     //game loop
//     void input(float delta_time_s);
//     void update(float delta_time_s);
//     void render();
// };

} //namespace game2d