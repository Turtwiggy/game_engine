#pragma once

//other project headers
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

//your project headers
#include "engine/resources/resource_manager.hpp"
#include "engine/graphics/shader.hpp"
#include "engine/graphics/texture.hpp"
#include "engine/mesh/primitives.hpp"

namespace game2d 
{

struct Transform
{
    glm::vec2 position = { 0.0f, 0.0f }; //in pixels, centered
    float angle = 0.0f;                 //in radians, about the origin, which is currently the centre
    glm::vec2 scale = { 1.0f, 1.0f };
    glm::vec3 colour = { 0.0f, 1.0f, 0.0f };
    //bool depress_on_hover = false;
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

void draw_sprite (  
    fightingengine::Shader& shader,
    unsigned int VAO,
    fightingengine::Texture2D& texture, 
    glm::vec2 position, 
    glm::vec2 size, 
    float rotate, 
    glm::vec3 color ); 

} //namespace game2d