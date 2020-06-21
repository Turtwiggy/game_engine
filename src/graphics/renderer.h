#pragma once

#include "window/game_window.h"

#include "3d/assimp_obj_loader.h"
#include "3d/camera.h"
#include "graphics/renderer_api.h"
#include "graphics/render_pass.h"
#include "graphics/shader.h"
#include "entities/transform.h"

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
        game_window& window;
        Camera& camera;
        std::vector<std::reference_wrapper<FGTransform>>& transforms;

        draw_scene_desc(std::vector<std::reference_wrapper<FGTransform>>& m, Camera& c, game_window& w)
            : transforms(m)
            , camera(c)
            , window(w)
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

        void shutdown();

    private:

        //temp opengl testing
        unsigned int texID;
        glm::vec3 cube_pos;

    public:
        // Stats
        struct Statistics
        {
            uint32_t DrawCalls = 0;
        };

    public:

        ImGuiContext* get_imgui_context() { return _imgui; }

        void init_opengl_and_imgui(const game_window& window);
        void init_models_and_shaders(std::vector<std::reference_wrapper<FGTransform>>& models);

        SDL_GLContext get_gl_context() { return gl_context; }

    private:
        //opengl
        SDL_GLContext gl_context;
        ImGuiContext* _imgui;
    };
}
