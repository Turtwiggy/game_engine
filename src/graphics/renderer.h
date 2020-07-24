#pragma once

#include "3d/fg_transform.hpp"
#include "3d/fg_object.hpp"
#include "3d/camera.hpp"
#include <game_state.hpp>
#include "graphics/renderer_api.h"
#include "graphics/render_pass.h"
#include "graphics/opengl/opengl_shader.h"
#include "window/game_window.h"

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
        GameWindow& window;
        Camera& camera;

        bool hdr = true;
        float exposure = 1.0f;

        draw_scene_desc(Camera& c, GameWindow& w, bool hdr, float exposure)
            : camera(c)
            , window(w)
            , hdr(hdr)
            , exposure(exposure)
        {
        }
    };

    class Renderer
    {
    public:
        static renderer_api::API get_api() { return renderer_api::get_api(); }

        void new_frame(SDL_Window* window);
        void end_frame(SDL_Window* window);

        void draw_pass(draw_scene_desc& desc, GameState state);

        unsigned int hdr_fbo();
        std::array<unsigned int, 2> hdr_colour_buffer(int width, int height, unsigned int hdr_fbo);
        void renderCube();
        void renderQuad();

        void shutdown();

    public:
        // Stats
        struct Statistics
        {
            uint32_t DrawCalls = 0;
        };

    public:

        ImGuiContext* get_imgui_context() { return _imgui; }

        void init_opengl_and_imgui(const GameWindow& window);
        void init_renderer(int screen_width, int screen_height);

        SDL_GLContext get_gl_context() { return gl_context; }

    private:
        //opengl
        SDL_GLContext gl_context;
        ImGuiContext* _imgui;
    };
}
