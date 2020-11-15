
//header
#include "engine/mesh/primitives.hpp"

//c++ standard library
#include <vector>

//other library headers
#include <GL/glew.h>

namespace fightingengine {

void render_mesh(std::shared_ptr<Mesh> mesh)
{
    //bind vao
    glBindVertexArray(mesh->vao);

    if (mesh->Indices.size() > 0)
    {
        glDrawElements(mesh->topology == TOPOLOGY::TRIANGLE_STRIP ? GL_TRIANGLE_STRIP : GL_TRIANGLES, mesh->Indices.size(), GL_UNSIGNED_INT, 0);
    }
    else
    {
        glDrawArrays(mesh->topology == TOPOLOGY::TRIANGLE_STRIP ? GL_TRIANGLE_STRIP : GL_TRIANGLES, 0, mesh->Positions.size());
    }
}

void render_mesh(Mesh& mesh)
{
    //bind vao
    glBindVertexArray(mesh.vao);

    if (mesh.Indices.size() > 0)
    {
        glDrawElements(mesh.topology == TOPOLOGY::TRIANGLE_STRIP ? GL_TRIANGLE_STRIP : GL_TRIANGLES, mesh.Indices.size(), GL_UNSIGNED_INT, 0);
    }
    else
    {
        glDrawArrays(mesh.topology == TOPOLOGY::TRIANGLE_STRIP ? GL_TRIANGLE_STRIP : GL_TRIANGLES, 0, mesh.Positions.size());
    }
}


namespace primitives {

// ---- plane

Plane::Plane(unsigned int x_segments, unsigned int y_segments)
{

    Positions = std::vector<glm::vec3> {
        glm::vec3(0.0f, 1.0f, 0.0f),
        glm::vec3(1.0f, 0.0f, 0.0f),
        glm::vec3(0.0f, 0.0f, 0.0f),

        glm::vec3(0.0f, 1.0f, 0.0f),
        glm::vec3(1.0f, 1.0f, 0.0f),
        glm::vec3(1.0f, 0.0f, 0.0f)
    };
    UV = std::vector<glm::vec2> {
        glm::vec2(0.0f, 1.0f),
        glm::vec2(1.0f, 0.0f),
        glm::vec2(0.0f, 0.0f),

        glm::vec2(0.0f, 1.0f),
        glm::vec2(1.0f, 1.0f),
        glm::vec2(1.0f, 0.0f)
    };
    Normals = std::vector<glm::vec3> {
        glm::vec3(0.0f, 0.0f, 1.0f),
        glm::vec3(0.0f, 0.0f, 1.0f),
        glm::vec3(0.0f, 0.0f, 1.0f),

        glm::vec3(0.0f, 0.0f, 1.0f),
        glm::vec3(0.0f, 0.0f, 1.0f),
        glm::vec3(0.0f, 0.0f, 1.0f)
    };

    // bool oddRow = false;

    // float dX = 1.0f / x_segments;
    // float dY = 1.0f / y_segments;

    // for (int y = 0; y <= y_segments; ++y)
    // {
    //     for (int x = 0; x <= x_segments; ++x)
    //     {
    //         Positions.push_back(glm::vec3(dX * x * 2.0f - 1.0f, dY * y * 2.0f - 1.0f, 0.0f));
    //         UV.push_back(glm::vec2(dX * x, 1.0f - y * dY));
    //         Normals.push_back(glm::vec3(0.0f, 0.0f, 1.0f));
    //     }
    // }

    // for (int y = 0; y < y_segments; ++y)
    // {
    //     if (!oddRow) // even rows: y == 0, y == 2; and so on
    //     {
    //         for (int x = 0; x <= x_segments; ++x)
    //         {
    //             Indices.push_back(y       * (x_segments + 1) + x);
    //             Indices.push_back((y + 1) * (x_segments + 1) + x);
    //         }
    //     }
    //     else
    //     {
    //         for (int x = x_segments; x >= 0; --x)
    //         {
    //             Indices.push_back((y + 1) * (x_segments + 1) + x);
    //             Indices.push_back(y       * (x_segments + 1) + x);
    //         }
    //     }
    //     oddRow = !oddRow;
    // }

    topology = TOPOLOGY::TRIANGLE_STRIP;
    Finalize();
}

// ---- cube

Cube::Cube() 
{
    Positions = std::vector<glm::vec3> {
        glm::vec3(-0.5f, -0.5f, -0.5f),
        glm::vec3( 0.5f,  0.5f, -0.5f), 
        glm::vec3( 0.5f, -0.5f, -0.5f), 
        glm::vec3( 0.5f,  0.5f, -0.5f), 
        glm::vec3(-0.5f, -0.5f, -0.5f),
        glm::vec3(-0.5f,  0.5f, -0.5f),

        glm::vec3(-0.5f, -0.5f,  0.5f),
        glm::vec3( 0.5f, -0.5f,  0.5f),
        glm::vec3( 0.5f,  0.5f,  0.5f),
        glm::vec3( 0.5f,  0.5f,  0.5f), 
        glm::vec3(-0.5f,  0.5f,  0.5f),
        glm::vec3(-0.5f, -0.5f,  0.5f),

        glm::vec3(-0.5f,  0.5f,  0.5f),
        glm::vec3(-0.5f,  0.5f, -0.5f),
        glm::vec3(-0.5f, -0.5f, -0.5f),
        glm::vec3(-0.5f, -0.5f, -0.5f),
        glm::vec3(-0.5f, -0.5f,  0.5f),
        glm::vec3(-0.5f,  0.5f,  0.5f),

        glm::vec3( 0.5f,  0.5f,  0.5f), 
        glm::vec3( 0.5f, -0.5f, -0.5f), 
        glm::vec3( 0.5f,  0.5f, -0.5f), 
        glm::vec3( 0.5f, -0.5f, -0.5f), 
        glm::vec3( 0.5f,  0.5f,  0.5f), 
        glm::vec3( 0.5f, -0.5f,  0.5f), 

        glm::vec3(-0.5f, -0.5f, -0.5f),
        glm::vec3( 0.5f, -0.5f, -0.5f),
        glm::vec3( 0.5f, -0.5f,  0.5f),
        glm::vec3( 0.5f, -0.5f,  0.5f), 
        glm::vec3(-0.5f, -0.5f,  0.5f),
        glm::vec3(-0.5f, -0.5f, -0.5f),

        glm::vec3(-0.5f,  0.5f, -0.5f),
        glm::vec3( 0.5f,  0.5f,  0.5f), 
        glm::vec3( 0.5f,  0.5f, -0.5f), 
        glm::vec3( 0.5f,  0.5f,  0.5f), 
        glm::vec3(-0.5f,  0.5f, -0.5f),
        glm::vec3(-0.5f,  0.5f,  0.5f),
    };
    UV = std::vector<glm::vec2> {
        glm::vec2(0.0f, 0.0f),
        glm::vec2(1.0f, 1.0f),
        glm::vec2(1.0f, 0.0f),
        glm::vec2(1.0f, 1.0f),
        glm::vec2(0.0f, 0.0f),
        glm::vec2(0.0f, 1.0f),

        glm::vec2(0.0f, 0.0f),
        glm::vec2(1.0f, 0.0f),
        glm::vec2(1.0f, 1.0f),
        glm::vec2(1.0f, 1.0f),
        glm::vec2(0.0f, 1.0f),
        glm::vec2(0.0f, 0.0f),

        glm::vec2(1.0f, 0.0f),
        glm::vec2(1.0f, 1.0f),
        glm::vec2(0.0f, 1.0f),
        glm::vec2(0.0f, 1.0f),
        glm::vec2(0.0f, 0.0f),
        glm::vec2(1.0f, 0.0f),

        glm::vec2(1.0f, 0.0f),
        glm::vec2(0.0f, 1.0f),
        glm::vec2(1.0f, 1.0f),
        glm::vec2(0.0f, 1.0f),
        glm::vec2(1.0f, 0.0f),
        glm::vec2(0.0f, 0.0f),

        glm::vec2(0.0f, 1.0f),
        glm::vec2(1.0f, 1.0f),
        glm::vec2(1.0f, 0.0f),
        glm::vec2(1.0f, 0.0f),
        glm::vec2(0.0f, 0.0f),
        glm::vec2(0.0f, 1.0f),

        glm::vec2(0.0f, 1.0f),
        glm::vec2(1.0f, 0.0f),
        glm::vec2(1.0f, 1.0f),
        glm::vec2(1.0f, 0.0f),
        glm::vec2(0.0f, 1.0f),
        glm::vec2(0.0f, 0.0f),
    };
    Normals = std::vector<glm::vec3>{
        glm::vec3( 0.0f,  0.0f, -1.0f),
        glm::vec3( 0.0f,  0.0f, -1.0f),
        glm::vec3( 0.0f,  0.0f, -1.0f),
        glm::vec3( 0.0f,  0.0f, -1.0f),
        glm::vec3( 0.0f,  0.0f, -1.0f),
        glm::vec3( 0.0f,  0.0f, -1.0f),

        glm::vec3( 0.0f,  0.0f,  1.0f),
        glm::vec3( 0.0f,  0.0f,  1.0f),
        glm::vec3( 0.0f,  0.0f,  1.0f),
        glm::vec3( 0.0f,  0.0f,  1.0f),
        glm::vec3( 0.0f,  0.0f,  1.0f),
        glm::vec3( 0.0f,  0.0f,  1.0f),

        glm::vec3(-1.0f,  0.0f,  0.0f),
        glm::vec3(-1.0f,  0.0f,  0.0f),
        glm::vec3(-1.0f,  0.0f,  0.0f),
        glm::vec3(-1.0f,  0.0f,  0.0f),
        glm::vec3(-1.0f,  0.0f,  0.0f),
        glm::vec3(-1.0f,  0.0f,  0.0f),

        glm::vec3( 1.0f,  0.0f,  0.0f),
        glm::vec3( 1.0f,  0.0f,  0.0f),
        glm::vec3( 1.0f,  0.0f,  0.0f),
        glm::vec3( 1.0f,  0.0f,  0.0f),
        glm::vec3( 1.0f,  0.0f,  0.0f),
        glm::vec3( 1.0f,  0.0f,  0.0f),

        glm::vec3( 0.0f, -1.0f,  0.0f),
        glm::vec3( 0.0f, -1.0f,  0.0f),
        glm::vec3( 0.0f, -1.0f,  0.0f),
        glm::vec3( 0.0f, -1.0f,  0.0f),
        glm::vec3( 0.0f, -1.0f,  0.0f),
        glm::vec3( 0.0f, -1.0f,  0.0f),

        glm::vec3( 0.0f,  1.0f,  0.0f),
        glm::vec3( 0.0f,  1.0f,  0.0f),
        glm::vec3( 0.0f,  1.0f,  0.0f),
        glm::vec3( 0.0f,  1.0f,  0.0f),
        glm::vec3( 0.0f,  1.0f,  0.0f),
        glm::vec3( 0.0f,  1.0f,  0.0f),
    };

    topology = TOPOLOGY::TRIANGLES;
    Finalize();
}

} //namespace primitives
    
} //namespace fightingengine