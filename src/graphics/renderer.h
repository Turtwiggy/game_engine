#pragma once

#include "window/game_window.h"

#include "graphics/shader.h"
#include "graphics/render_pass.h"

#include <SDL2/SDL.h>

#include <array>
#include <string_view>
#include <memory>

struct shader_definition
{
    const std::string_view name;
    const std::string_view vertexShaderName;
    const std::string_view fragmentShaderName;
};

namespace fightinggame {

    constexpr std::array shaders{
        NULL
        //ShaderDefinition {"DebugLine", "vs_line", "fs_line"},
        //ShaderDefinition {"DebugLineInstanced", "vs_line_instanced", "fs_line"},
        //ShaderDefinition {"Terrain", "vs_terrain", "fs_terrain"},
        //ShaderDefinition {"Object", "vs_object", "fs_object"},
        //ShaderDefinition {"ObjectInstanced", "vs_object_instanced", "fs_object"},
        //ShaderDefinition {"Water", "vs_water", "fs_water"},
    };

    class renderer
    {
    public:
        enum class api
        {
            OpenGL,
        };

        struct draw_scene_desc
        {
            graphics::render_pass view_id;
            int height;
            int width;
        };

        renderer() = delete;
        explicit renderer(const game_window* window, bool vsync);

        virtual ~renderer();

        void configure_view(graphics::render_pass view_id, uint16_t width, uint16_t height) const;
        void draw_scene(const draw_scene_desc& desc) const;
        void draw_pass(const draw_scene_desc& desc) const;
        void frame(SDL_Window* window);

        unsigned int loadTexture(char const* path);

    private:

        //temp opengl testing
        //unsigned int texId;
        //std::unique_ptr<Shader> flatColorShader;
    };
}