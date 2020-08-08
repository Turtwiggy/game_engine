#pragma once

#include "3d/fg_transform.hpp"
#include "3d/fg_object.hpp"
#include "3d/camera.hpp"
#include <game_state.hpp>
#include "graphics/renderer_api.h"
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
        //graphics::render_pass view_id;
        GameWindow& window;
        Camera& camera;

        float exposure = 1.0f;

        draw_scene_desc(Camera& c, GameWindow& w, float exposure)
            : camera(c)
            , window(w)
            , exposure(exposure)
        {
        }
    };

    class RendererImpl
    {
    public:
        virtual void init(int screen_width, int screen_height) = 0;
        virtual void draw_pass(draw_scene_desc& desc, const GameState& state) = 0;
        virtual void resize(int width, int height) = 0;

        // renderQuad() renders a 1x1 XY quad in NDC
        // -----------------------------------------
        unsigned int quadVAO = 0;
        unsigned int quadVBO;
        void renderQuad()
        {
            if (quadVAO == 0)
            {
                float quadVertices[] = {
                    // positions        // texture Coords
                    -1.0f,  1.0f, 0.0f, 0.0f, 1.0f,
                    -1.0f, -1.0f, 0.0f, 0.0f, 0.0f,
                     1.0f,  1.0f, 0.0f, 1.0f, 1.0f,
                     1.0f, -1.0f, 0.0f, 1.0f, 0.0f,
                };
                // setup plane VAO
                glGenVertexArrays(1, &quadVAO);
                glGenBuffers(1, &quadVBO);
                glBindVertexArray(quadVAO);
                glBindBuffer(GL_ARRAY_BUFFER, quadVBO);
                glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), &quadVertices, GL_STATIC_DRAW);
                glEnableVertexAttribArray(0);
                glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
                glEnableVertexAttribArray(1);
                glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
            }
            glBindVertexArray(quadVAO);
            glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
            glBindVertexArray(0);
        }
    };

    class Renderer
    {
    public:
        static RendererAPI::API get_api() { return RendererAPI::get_api(); }

        void new_frame(SDL_Window* window);
        void end_frame(SDL_Window* window);

        void shutdown();

        std::unique_ptr<RendererImpl> renderer_impl;

    public:

    public:
        void init_opengl_and_imgui(const GameWindow& window);
        void init_renderer(int screen_width, int screen_height);

        ImGuiContext* get_imgui_context() { return _imgui; }
        SDL_GLContext get_gl_context() { return gl_context; }

    private:
        //opengl
        SDL_GLContext gl_context;
        ImGuiContext* _imgui;
    };
}
