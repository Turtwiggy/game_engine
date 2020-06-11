#pragma once

#include "window/game_window.h"
#include "base.h"

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

    struct draw_scene_desc
    {
        graphics::render_pass view_id;
        int height = 0;
        int width = 0;
        Camera& camera;
        std::vector<std::reference_wrapper<Model>>& models;

        draw_scene_desc(std::vector<std::reference_wrapper<Model>>& models, Camera& c)
            : models(models)
            , camera(c)
        {
        }
    };

    class renderer
    {
    public:
        static renderer_api::API get_api() { return renderer_api::GetAPI(); }

        void new_frame(SDL_Window* window);
        void end_frame(SDL_Window* window);

        void draw_pass(draw_scene_desc& desc);
        void flush();
        void flush_and_reset();

        void draw_model(Model& m, glm::vec3& position, glm::vec3& size /*, float rotation, const Ref<Texture>& texture*/);

        void shutdown();

    private:
        void render_square(Shader& shader); //temporary

    private:

        //temp opengl testing
        unsigned int texID;

        glm::vec3 cube_pos;

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

    public:

        ImGuiContext* get_imgui_context() { return _imgui; }

        void init_opengl_and_imgui(const game_window& window);
        void init_models_and_shaders();

        SDL_GLContext get_gl_context() { return gl_context; }

    private:
        //opengl
        SDL_GLContext gl_context;
        ImGuiContext* _imgui;

    };
}
