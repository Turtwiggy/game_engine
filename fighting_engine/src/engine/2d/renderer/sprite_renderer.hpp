#pragma once

#include "engine/renderer/texture.hpp"
#include "engine/renderer/shader.hpp"
using namespace fightingengine;

#include "glm/glm.hpp"

struct SpriteHandle
{
    glm::vec2 offset;
    glm::vec4 base_colour = { 1.0f, 1.0f, 1.0f, 1.0f };
};

struct TilemapPosition
{
    glm::vec2 pos;
};

struct RenderDescriptor
{
    glm::vec2 position;                 //in pixels, centered
    glm::vec4 colour    = { 1,1,1,1 };  //linear colour, everything's fine
    float angle         = 0;            //in radians, about the origin, which is currently the centre
    glm::vec2 scale     = { 1,1 };
    bool depress_on_hover = false;
};

//This is simpler than it sounds, where you have your texture co-ordinates, rather than doing from 0 to 1,
//you pass into your shader using a frame number, as well as the number of frames
//and calculate with that the texture coords to render. For example if you have a spritesheet that has
// a row of 8 sprites and you want the 4th frame rendered, your u coordinates would be between 4/8 and 5/8

class SpriteRenderer
{
public:
    SpriteRenderer(Shader& shader);
    ~SpriteRenderer();

    void DrawSprite(Texture2D& texture, const RenderDescriptor& desc, const SpriteHandle& handle);
private:
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
