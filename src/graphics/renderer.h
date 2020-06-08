#pragma once

#include "window/game_window.h"

#include "3d/assimp_obj_loader.h"
#include "3d/camera.h"
#include "graphics/renderer_api.h"
#include "graphics/render_pass.h"
#include "graphics/shader.h"

#include <SDL2/SDL.h>
#include <imgui.h>
#include <glm/glm.hpp>
#include <glm/vec3.hpp>

#include <array>
#include <string_view>
#include <memory>

namespace fightinggame {

    class renderer
    {
    public:

        struct draw_scene_desc
        {
            graphics::render_pass view_id;
            int height = 0;
            int width = 0;
            Camera camera;
            Model &main_character;

            draw_scene_desc(Model &m1)
                : main_character(m1)
            {
            }
        };

        renderer() = delete;
        explicit renderer(const game_window& window, bool vsync);

        static renderer_api::API get_api() { return renderer_api::GetAPI(); }

        void draw_pass(const draw_scene_desc& desc);
        void new_frame(SDL_Window* window);
        void end_frame(SDL_Window* window);

        void render_cube(glm::vec3& position, glm::vec3& size /*, float rotation, const Ref<Texture>& texture*/);


        ImGuiContext* get_imgui_context() { return _imgui; }

        void init_opengl_and_imgui(const game_window& window);
        SDL_GLContext get_gl_context() { return gl_context; }

        void shutdown();

    private:
        void render_square(Shader& shader);

    private:

        //temp opengl testing
        std::unique_ptr<Shader> shader;
        unsigned int texID;

        //opengl
        SDL_GLContext gl_context;
        ImGuiContext* _imgui;

    public:
        // Stats
        struct Statistics
        {
            uint32_t DrawCalls = 0;
            uint32_t CubeCount = 0;

            uint32_t GetTotalVertexCount() { return CubeCount * 30; }
            uint32_t GetTotalIndexCount() { return CubeCount * 36; }
        };
        static void reset_stats();
        static Statistics get_stats();
    };
}
