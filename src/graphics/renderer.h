#pragma once

#include "window/game_window.h"

#include "graphics/renderer_api.h"
#include "graphics/render_pass.h"
#include "graphics/shader.h"
#include "3d/camera.h"

#include <SDL2/SDL.h>
#include <imgui.h>
#include <glm/glm.hpp>
#include <glm/vec3.hpp>

#include <array>
#include <string_view>
#include <memory>

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

        struct draw_scene_desc
        {
            graphics::render_pass view_id;
            int height;
            int width;
            std::shared_ptr<Camera> camera;
        };

        renderer() = delete;
        explicit renderer(const game_window* window, bool vsync);
        virtual ~renderer();

        static renderer_api::API get_api() { return renderer_api::GetAPI(); }

        void draw_pass(const draw_scene_desc& desc);
        void new_frame(SDL_Window* window);
        void end_frame(SDL_Window* window);

        void render_cube(glm::vec3& position, glm::vec3& size /*, float rotation, const Ref<Texture>& texture*/);


        ImGuiContext* get_imgui_context() { return _imgui; }

        void init_opengl(const game_window* window);
        SDL_GLContext get_gl_context() { return gl_context; }

    private:
        void render_square(Shader* shader);
        void setup_square_buffer(unsigned int VAO, unsigned int VBO, unsigned int EBO);

        void loadTexture(const std::string& path);

    private:

        //temp opengl testing
        unsigned int texId;
        std::unique_ptr<Shader> flatColorShader;
        //-end temp

        //opengl
        SDL_GLContext gl_context;
        ImGuiContext* _imgui;

        uint32_t m_RendererID;

    public:
        // Stats
        struct Statistics
        {
            uint32_t DrawCalls = 0;
            uint32_t QuadCount = 0;

            uint32_t GetTotalVertexCount() { return QuadCount * 4; }
            uint32_t GetTotalIndexCount() { return QuadCount * 6; }
        };
        static void reset_stats();
        static Statistics get_stats();
    };
}