#pragma once

#include "window/game_window.h"

#include "graphics/shader.h"
#include "graphics/render_pass.h"

#include <SDL2/SDL.h>
#include <imgui.h>

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
        void init_opengl(const game_window* window);

        virtual ~renderer();

        void configure_view(graphics::render_pass view_id, uint16_t width, uint16_t height) const;
        void draw_scene(const draw_scene_desc& desc) const;
        void draw_pass(const draw_scene_desc& desc) const;
        void new_frame(SDL_Window* window);
        void end_frame(SDL_Window* window);

        unsigned int loadTexture(char const* path);

        ImGuiContext* get_imgui_context() { return _imgui; }
        SDL_GLContext get_gl_context() { return gl_context; }

    private:
        void render_at_position(Shader* shader);

    private:

        //temp opengl testing
        //std::array<float, 12> vertices;
        //std::array<unsigned int, 6> indices;
        unsigned int VAO;
        unsigned int texId;

        //unsigned int texId;
        std::unique_ptr<Shader> flatColorShader;

        //opengl
        SDL_GLContext gl_context;
        ImGuiContext* _imgui;
    };
}