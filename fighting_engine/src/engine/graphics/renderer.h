#pragma once

#include "engine/3d/camera.hpp"
#include "engine/3d/game_object_3d.hpp"
#include "engine/core/window/game_window.h"
#include "engine/graphics/renderer_api.h"
#include "engine/util/singleton.h"
#include "platform/opengl/shader.hpp"

#include <SDL2/SDL.h>
#include <imgui.h>
#include <glm/glm.hpp>
#include <glm/vec3.hpp>

#include <array>
#include <string_view>
#include <memory>

namespace fightingengine {

    struct RenderDescriptor
    {
        //graphics::render_pass view_id;
        const GameWindow& window;
        const Camera& camera;

        //Objects to render
        const std::vector<std::reference_wrapper<GameObject3D>>& objects;

        RenderDescriptor(const Camera& c, const GameWindow& w, const std::vector<std::reference_wrapper<GameObject3D>>& o)
            : camera(c)
            , window(w)
            , objects(o)
        {
        }
    };

    class RendererImpl
    {
    public:
        virtual void init(int screen_width, int screen_height) = 0;
        virtual void draw_pass(RenderDescriptor& desc) = 0;
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

    class Renderer : public Singleton<Renderer>
    {
    public:
        void init_renderer(int screen_width, int screen_height);

        static RendererAPI::API get_api() { return RendererAPI::get_api(); }

        void new_frame(SDL_Window* window);
        void end_frame(SDL_Window* window);
        void resize(int width, int height) { renderer_impl->resize(width, height); };

        void shutdown();

        std::unique_ptr<RendererImpl> renderer_impl;

    public:
        void init_opengl(const GameWindow& window);
        SDL_GLContext get_gl_context() { return gl_context; }


    private:
        SDL_GLContext gl_context;

    };
}
