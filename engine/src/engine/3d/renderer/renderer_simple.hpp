#pragma once

//c++ standard lib headers
#include <memory>
#include <vector>

//other library files
#include <glm/glm.hpp>
#include <glm/gtx/transform.hpp>
#include <glm/gtc/matrix_transform.hpp>

//your project headers
#include "engine/core/maths/random.hpp"
#include "engine/core/application.hpp"
#include "engine/3d/camera/camera.hpp"
#include "engine/3d/camera/fly_camera.hpp"
#include "engine/graphics/framebuffer.hpp"
#include "engine/scene/scene_node.hpp"
#include "engine/scene/background.hpp"
#include "engine/mesh/primitives.hpp"

namespace fightingengine
{

class RendererSimple
{
public:
    RendererSimple( RandomState& rnd, int width, int height );
    
    void update(
        float delta_time, 
        FlyCamera& camera, 
        RandomState& rnd, 
        const std::vector<glm::vec3>& cube_pos );

    int get_draw_calls();

private:
    void render_scene( 
        const glm::mat4& view_projection, 
        const std::vector<glm::vec3>& cube_pos );

    void draw_skybox( const glm::mat4& view_projection );

private:

    //rendering shaders / meshes available
    Shader flat_shader_;
    std::shared_ptr<Mesh> cube;
    std::shared_ptr<Mesh> plane;

    //skybox
    Background* background;
    std::shared_ptr<TextureCube> cubemap_;

    //shadow mapping
    int shadowmap_width_ = 1024;
    int shadowmap_height_ = 1024;
    int shadowmap_fbo_;
    Texture2D shadowmap_texture_;
    Shader shadowmap_shader_;
    glm::vec3 light_pos_ = {0.0f, 0.0f, 0.0f};
    
    //renderer info
    int draw_calls_ = 0;
    int width_, height_ = 0;
};

} //namespace fightingengine






// unsigned int sphereVAO = 0;
// unsigned int indexCount;
// void renderSphere();

// void RendererSimple::renderSphere()
// {
//     draw_calls_ += 1;

//     if (sphereVAO == 0)
//     {
//         glGenVertexArrays(1, &sphereVAO);

//         unsigned int vbo, ebo;
//         glGenBuffers(1, &vbo);
//         glGenBuffers(1, &ebo);

//         std::vector<glm::vec3> positions;
//         std::vector<glm::vec2> uv;
//         std::vector<glm::vec3> normals;
//         std::vector<unsigned int> indices;

//         const unsigned int X_SEGMENTS = 64;
//         const unsigned int Y_SEGMENTS = 64;
//         const float PI = 3.14159265359;
//         for (unsigned int y = 0; y <= Y_SEGMENTS; ++y)
//         {
//             for (unsigned int x = 0; x <= X_SEGMENTS; ++x)
//             {
//                 float xSegment = (float)x / (float)X_SEGMENTS;
//                 float ySegment = (float)y / (float)Y_SEGMENTS;
//                 float xPos = std::cos(xSegment * 2.0f * PI) * std::sin(ySegment * PI);
//                 float yPos = std::cos(ySegment * PI);
//                 float zPos = std::sin(xSegment * 2.0f * PI) * std::sin(ySegment * PI);

//                 positions.push_back(glm::vec3(xPos, yPos, zPos));
//                 uv.push_back(glm::vec2(xSegment, ySegment));
//                 normals.push_back(glm::vec3(xPos, yPos, zPos));
//             }
//         }

//         bool oddRow = false;
//         for (unsigned int y = 0; y < Y_SEGMENTS; ++y)
//         {
//             if (!oddRow) // even rows: y == 0, y == 2; and so on
//             {
//                 for (unsigned int x = 0; x <= X_SEGMENTS; ++x)
//                 {
//                     indices.push_back(y       * (X_SEGMENTS + 1) + x);
//                     indices.push_back((y + 1) * (X_SEGMENTS + 1) + x);
//                 }
//             }
//             else
//             {
//                 for (int x = X_SEGMENTS; x >= 0; --x)
//                 {
//                     indices.push_back((y + 1) * (X_SEGMENTS + 1) + x);
//                     indices.push_back(y       * (X_SEGMENTS + 1) + x);
//                 }
//             }
//             oddRow = !oddRow;
//         }
//         indexCount = indices.size();

//         std::vector<float> data;
//         for (std::size_t i = 0; i < positions.size(); ++i)
//         {
//             data.push_back(positions[i].x);
//             data.push_back(positions[i].y);
//             data.push_back(positions[i].z);
//             if (uv.size() > 0)
//             {
//                 data.push_back(uv[i].x);
//                 data.push_back(uv[i].y);
//             }
//             if (normals.size() > 0)
//             {
//                 data.push_back(normals[i].x);
//                 data.push_back(normals[i].y);
//                 data.push_back(normals[i].z);
//             }
//         }
//         glBindVertexArray(sphereVAO);
//         glBindBuffer(GL_ARRAY_BUFFER, vbo);
//         glBufferData(GL_ARRAY_BUFFER, data.size() * sizeof(float), &data[0], GL_STATIC_DRAW);
//         glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
//         glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), &indices[0], GL_STATIC_DRAW);
//         float stride = (3 + 2 + 3) * sizeof(float);
//         glEnableVertexAttribArray(0);
//         glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, stride, (void*)0);
//         glEnableVertexAttribArray(1);
//         glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, stride, (void*)(3 * sizeof(float)));
//         glEnableVertexAttribArray(2);
//         glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, stride, (void*)(5 * sizeof(float)));
//     }

//     glBindVertexArray(sphereVAO);
//     glDrawElements(GL_TRIANGLE_STRIP, indexCount, GL_UNSIGNED_INT, 0);
// }
