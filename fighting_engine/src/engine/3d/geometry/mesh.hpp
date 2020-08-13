#pragma once

#include "3d/geometry/triangle.hpp"
#include "3d/geometry/vertex.hpp"
#include "graphics/opengl/texture.hpp"
#include "graphics/opengl/shader.hpp"
#include "graphics/colour.hpp"

#include <string>
#include <vector>
#include <functional>

namespace fightingengine {

    class Mesh {
    public:
        // mesh data
        std::vector<Vertex>       vertices;
        std::vector<unsigned int>   indices;
        std::vector<Texture2D>      textures;
        std::vector<Triangle>     triangles;
        std::string                 name;
        ColourVec4f                    colour;

        Mesh
        (
            std::vector<Vertex>   vertices,
            std::vector<unsigned int> indices,
            std::vector<Texture2D>  textures,
            ColourVec4f                colour,
            std::string             name
        );

        void draw(Shader& shader, int texture = -1);
        void setup_mesh();

    private:
        //  render data
        unsigned int VAO, VBO, EBO;
    };
}
