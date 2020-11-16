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



//This is simpler than it sounds, where you have your texture co-ordinates, rather than doing from 0 to 1,
//you pass into your shader using a frame number, as well as the number of frames
//and calculate with that the texture coords to render. For example if you have a spritesheet that has
// a row of 8 sprites and you want the 4th frame rendered, your u coordinates would be between 4/8 and 5/8


} //namespace fightingengine
