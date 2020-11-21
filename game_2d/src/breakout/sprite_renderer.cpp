
//header
#include "breakout/sprite_renderer.hpp"

//engine project headers
#include "engine/graphics/render_command.hpp"

namespace game2d {


void update_renderer(
    Shader& shader, 
    Mesh& mesh, 
    int screen_width, 
    int screen_height, 
    Breakout& breakout, 
    GameObject& player )
{
    RenderCommand::clear();

    // draw a background
    draw_sprite (shader, mesh, ResourceManager::get_texture("background"), 
        {0.0f, 0.0f},                   //pos
        {screen_width, screen_height},  //size
        0.0f,                           //angle
        {1.0f, 1.0f, 1.0f}              //colour
    );

    // draw scene
    if(breakout.state == GameState::GAME_ACTIVE)
    {   
        for(auto brick : breakout.levels[0].bricks)
        {
            if(!brick.destroyed)
                draw_sprite( shader, mesh, brick );
        }
    }

    // draw player
    draw_sprite ( shader, mesh, player );        

}

void draw_sprite ( Shader& shader, Mesh& mesh, GameObject& game_object )
{
    draw_sprite( shader, mesh, game_object.texture, game_object.transform );
}

void draw_sprite ( Shader& shader, Mesh& mesh, Texture2D* texture, Transform& t )
{
    draw_sprite( shader, mesh, texture, t.position, t.scale, t.angle, t.colour);
}

void draw_sprite( Shader& shader, Mesh& mesh, Texture2D* texture, glm::vec2 position, glm::vec2 size, float angle, glm::vec3 color )
{
    if(texture == nullptr)
    {
        printf("(draw_sprite error) texture passed in is null! ");
        return;
    }

    shader.bind();

    glm::mat4 model = glm::mat4(1.0f);
    model = glm::translate( model, glm::vec3(position, 0.0f) );  
    // move origin of rotation to center of quad
    model = glm::translate( model, glm::vec3(0.5f * size.x, 0.5f * size.y, 0.0f) ); 
    // then rotate
    model = glm::rotate( model, glm::radians(angle), glm::vec3(0.0f, 0.0f, 1.0f) ); 
    // move origin back
    model = glm::translate( model, glm::vec3(-0.5f * size.x, -0.5f * size.y, 0.0f) ); 
    // then scale
    model = glm::scale( model, glm::vec3(size, 1.0f) ); 

    shader.set_mat4( "model", model );
    shader.set_vec3( "spriteColor", color );

    texture->Bind();

    render_mesh(mesh);

    texture->Unbind();
};

} //namespace game2d