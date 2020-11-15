#pragma once

//other library headers
#include <glm/glm.hpp>

//your project headers
#include "engine/graphics/texture.hpp"
#include "engine/graphics/shader.hpp"
#include "engine/graphics/colour.hpp"
#include "engine/mesh/primitives.hpp"

namespace fightingengine
{
    
struct SpriteHandle
{
    glm::ivec2 offset;
    ColourVec4f colour;
};

struct TilemapPosition
{
    glm::ivec2 pos;
};

struct Transform
{
    glm::vec2 position = { 0.0f, 0.0f }; //in pixels, centered
    float angle = 0.0f;                 //in radians, about the origin, which is currently the centre
    glm::vec2 scale = { 1.0f, 1.0f };
    ColourVec4f colour;
    //bool depress_on_hover = false;
};

//This is simpler than it sounds, where you have your texture co-ordinates, rather than doing from 0 to 1,
//you pass into your shader using a frame number, as well as the number of frames
//and calculate with that the texture coords to render. For example if you have a spritesheet that has
// a row of 8 sprites and you want the 4th frame rendered, your u coordinates would be between 4/8 and 5/8

struct SpriteRendererData
{
    Texture2D    spritesheet;
    std::string  spritesheet_name;
    primitives::Plane plane;
};

void init_sprite_renderer(SpriteRendererData& data)
{
    //data.spritesheet = ResourceManager::load_texture(("assets/" + spritesheet_name).c_str(), spritesheet_name, true, false);
    data.plane = primitives::Plane(2, 2);
};

void draw_sprite(const SpriteRendererData& data)
{

};

} //namespace fightingengine
