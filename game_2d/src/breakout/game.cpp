
//header
#include "breakout/game.hpp"



namespace game2d 
{

void init_breakout(BreakoutData& b, const int& width, const int& height)
{   
    using namespace fightingengine;

    Shader sprite_shader = ResourceManager::load_shader( 
        "assets/shaders/2d/", 
        {"sprite.vert", "sprite.frag"}, 
        "sprite_texture" 
    );

    glm::mat4 projection = glm::ortho(0.0f, static_cast<float>(width), static_cast<float>(height), 0.0f, -1.0f, 1.0f);
    
    sprite_shader.set_int("image", 0);
    sprite_shader.set_mat4("projection", projection);

    //load a texture
    Texture2D tex = ResourceManager::load_texture("assets/textures/marble.jpg", "marble");

    //take a copy (this is probs bad)
    b.sprite_shader = sprite_shader;
    b.sprite_texture = tex;
    b.projection = projection;
    b.state = BreakoutGameState::GAME_MENU;
}


} //namespace game2d