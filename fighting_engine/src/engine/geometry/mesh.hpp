#pragma once

#include "engine/3d/geometry/triangle.hpp"
#include "engine/3d/geometry/vertex.hpp"
#include "engine/renderer/colour.hpp"
#include "engine/renderer/texture.hpp"
#include "engine/renderer/shader.hpp"

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
