#pragma once

#include "engine/core/game_window.h"
#include "engine/renderer/shader.hpp"
#include "engine/scene/game_object_3d.hpp"
#include "engine/3d/camera.hpp"

#include <glm/glm.hpp>
#include <glm/vec3.hpp>

#include <vector>

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
}

//// renderQuad() renders a 1x1 XY quad in NDC
//// -----------------------------------------
//unsigned int quadVAO = 0;
//unsigned int quadVBO;
//void renderQuad()
//{
//    if (quadVAO == 0)
//    {
//        float quadVertices[] = {
//            // positions        // texture Coords
//            -1.0f,  1.0f, 0.0f, 0.0f, 1.0f,
//            -1.0f, -1.0f, 0.0f, 0.0f, 0.0f,
//             1.0f,  1.0f, 0.0f, 1.0f, 1.0f,
//             1.0f, -1.0f, 0.0f, 1.0f, 0.0f,
//        };
//        // setup plane VAO
//        glGenVertexArrays(1, &quadVAO);
//        glGenBuffers(1, &quadVBO);
//        glBindVertexArray(quadVAO);
//        glBindBuffer(GL_ARRAY_BUFFER, quadVBO);
//        glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), &quadVertices, GL_STATIC_DRAW);
//        glEnableVertexAttribArray(0);
//        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
//        glEnableVertexAttribArray(1);
//        glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
//    }
//    glBindVertexArray(quadVAO);
//    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
//    glBindVertexArray(0);
//}
