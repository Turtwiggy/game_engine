#pragma once

//other library headers
#include <glm/glm.hpp>

//your project headers
#include "engine/graphics/texture.hpp"
#include "engine/graphics/shader.hpp"
#include "engine/graphics/colour.hpp"

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

struct RenderDescriptor
{
    glm::vec2 position;         //in pixels, centered
    ColourVec4f colour;
    float angle = 0;            //in radians, about the origin, which is currently the centre
    glm::vec2 scale = { 1,1 };
    //bool depress_on_hover = false;
};

//This is simpler than it sounds, where you have your texture co-ordinates, rather than doing from 0 to 1,
//you pass into your shader using a frame number, as well as the number of frames
//and calculate with that the texture coords to render. For example if you have a spritesheet that has
// a row of 8 sprites and you want the 4th frame rendered, your u coordinates would be between 4/8 and 5/8

class SpriteRenderer
{
public:
    SpriteRenderer(Shader& shader, std::string sprite_sheet_name);
    ~SpriteRenderer();

    void draw_sprite(Texture2D& texture, const RenderDescriptor& desc, const SpriteHandle& handle);
    void render();
private:
    Texture2D    spritesheet;
    std::string  spritesheet_name;
    Shader       shader;
    unsigned int quadVAO;
};

//struct sprite_renderer
//{
//    std::vector<std::pair<sprite_handle, render_descriptor>> next_renderables;
//    texture sprite_sheet;
//
//    sprite_renderer();
//
//    void add(const sprite_handle& handle, const render_descriptor& descriptor);
//    void render(render_window& window, const camera& cam);
//};

} //namespace fightingengine
