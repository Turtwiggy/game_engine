#pragma once

//your project headers (engine)
#include "engine/resources/resource_manager.hpp"
#include "engine/graphics/shader.hpp"
#include "engine/graphics/texture.hpp"
#include "engine/mesh/primitives.hpp"

//your prject headers (game)
#include "breakout/game.hpp"

namespace game2d {

//This is simpler than it sounds, where you have your texture co-ordinates, rather than doing from 0 to 1,
//you pass into your shader using a frame number, as well as the number of frames
//and calculate with that the texture coords to render. For example if you have a spritesheet that has
// a row of 8 sprites and you want the 4th frame rendered, your u coordinates would be between 4/8 and 5/8

void draw_sprite ( Shader& shader, Mesh& mesh, GameObject& game_object  );
void draw_sprite ( Shader& shader, Mesh& mesh, Texture2D* texture, Transform& t );
void draw_sprite (  
    Shader&     shader,
    Mesh&       mesh,
    Texture2D*  texture, 
    glm::vec2   position, 
    glm::vec2   size, 
    float       angle, 
    glm::vec3   color ); 

} //namespace game2d