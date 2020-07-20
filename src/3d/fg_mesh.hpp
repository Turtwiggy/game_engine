#pragma once

#include "3d/fg_vertex.hpp"
#include "graphics/texture.h"
#include "graphics/opengl/opengl_shader.h"

#include "glm/glm.hpp"
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

#include <string>
#include <vector>
#include <functional>

namespace fightinggame {

    class FGMesh {
    public:
        // mesh data
        std::vector<FGVertex>                   vertices;
        std::vector<unsigned int>               indices;
        std::vector<std::shared_ptr<Texture2D>> textures;
        std::string                             name;

        FGMesh
        (
            std::vector<FGVertex> vertices,
            std::vector<unsigned int> indices,
            std::vector<std::shared_ptr<Texture2D>> textures,
            std::string name
        );

        void draw(Shader& shader);
        void setupMesh();

    private:
        //  render data
        unsigned int VAO, VBO, EBO;
    };
}
