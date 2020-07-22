#pragma once

#include "3d/fg_vertex.hpp"
#include "3d/fg_texture.hpp"
#include <3d/fg_colour.hpp>
#include "graphics/opengl/opengl_shader.h"

#include <string>
#include <vector>
#include <functional>

namespace fightinggame {

    class FGMesh {
    public:
        // mesh data
        std::vector<FGVertex>       vertices;
        std::vector<unsigned int>   indices;
        std::vector<Texture2D>      textures;
        std::string                 name;
        FGColour                    colour;

        FGMesh
        (
            std::vector<FGVertex>   vertices,
            std::vector<unsigned int> indices,
            std::vector<Texture2D>  textures,
            FGColour                colour,
            std::string             name
        );

        void draw(Shader& shader);
        void setup_mesh();

    private:
        //  render data
        unsigned int VAO, VBO, EBO;
    };
}
