#include "renderer.h"

namespace fightingengine
{
    struct RenderData
    {
        //textures
        unsigned int wood_texture;
        unsigned int second_texture;
    };
    static RenderData s_Data;

    //void Renderer::init_renderer(int screen_width, int screen_height)
    //{
    //    // load textures
    //    // -------------
    //    s_Data.wood_texture = TextureFromFile("BambooWall_1K_albedo.jpg", "assets/textures/Bamboo", false);
    //    s_Data.second_texture = TextureFromFile("container.jpg", "assets/textures", false);

    //    //GPU Info
    //    printf("OpenGL version supported by this platform (%s): \n", glGetString(GL_VERSION));
    //    int params[1];
    //    glGetIntegerv(GL_MAX_SHADER_STORAGE_BUFFER_BINDINGS, params);
    //    printf("GPU INFO: Max shader storage buffer bindings: %i \n", params[0]);
    //    //GL_MAX_SHADER_STORAGE_BUFFER_BINDINGS = 96
    //    //GL_MAX_SHADER_STORAGE_BLOCK_SIZE = 2147483647
    //    //GL_MAX_VERTEX_SHADER_STORAGE_BLOCKS = 16
    //    //GL_MAX_FRAGMENT_SHADER_STORAGE_BLOCKS = 16
    //    //GL_MAX_GEOMETRY_SHADER_STORAGE_BLOCKS = 16
    //    //GL_MAX_TESS_CONTROL_SHADER_STORAGE_BLOCKS = 16
    //    //GL_MAX_TESS_EVALUATION_SHADER_STORAGE_BLOCKS = 16
    //    //GL_MAX_COMPUTE_SHADER_STORAGE_BLOCKS = 16
    //    //GL_MAX_COMBINED_SHADER_STORAGE_BLOCKS = 96

    //    // draw as wireframe
    //    //glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

    //    //RenderCommand::init();
    //    //RenderCommand::set_viewport(0, 0, screen_width, screen_height);
    //}

}
