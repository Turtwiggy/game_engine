#pragma once

#include <string>
#include <vector>
#include <functional>

#include "engine/graphics/triangle.hpp"
#include "engine/graphics/vertex.hpp"
#include "engine/graphics/colour.hpp"
#include "engine/graphics/texture.hpp"
#include "engine/graphics/shader.hpp"

namespace fightingengine {

    class Mesh {
    public:
        std::string               name;
        // mesh data
        std::vector<Vertex>       vertices;
        std::vector<unsigned int> indices;
        std::vector<Texture2D>    textures;
        ColourVec4f               colour;

        Mesh
        (
            std::string               name,
            std::vector<Vertex>       vertices,
            std::vector<unsigned int> indices,
            std::vector<Texture2D>    textures,
            ColourVec4f               colour
        );

        //binds the VAO, draws the triangles
        void draw(Shader& shader);

        // initializes the VAO, VBO, and EBO
        void setup_mesh();

    private:
        //  render data
        unsigned int VAO, VBO, EBO;
    };
}
