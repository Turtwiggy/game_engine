#pragma once

//c++ standard lib headers
#include <vector>

//other project headers
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

//your project headers
#include "engine/graphics/texture.hpp"
using namespace fightingengine;

namespace game2d 
{

struct BreakoutTransform
{
    glm::vec2 position = { 0.0f, 0.0f }; //in pixels, centered
    float angle = 0.0f;                  //in degrees
    glm::vec2 scale = { 1.0f, 1.0f };
    glm::vec3 colour = { 0.0f, 1.0f, 0.0f };
};

struct BreakoutGameObject
{
    BreakoutTransform transform;
    glm::vec2 velocity = { 0.0f, 0.0f };
    Texture2D texture;

    //gameobject flags... 
    //note: avoid adding these, or come up with a better system
    //more flags is a 2^n of configurations of testing to make sure everything
    bool is_solid = false;
    bool destroyed = false;
};

struct BreakoutGameLevel
{
    std::vector<BreakoutGameObject> bricks;
};

enum class BreakoutGameState 
{
   GAME_ACTIVE,
   GAME_MENU,
   GAME_WIN
};

struct BreakoutData
{
    BreakoutGameState state;
};

} //namespace game2d