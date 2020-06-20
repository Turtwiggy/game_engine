#pragma once

#include "graphics/shader.h"
#include "graphics/texture.h"
#include "base.h"

//#include "assimp/"
#include "glm/glm.hpp"
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

#include <string>
#include <vector>
#include <functional>

namespace fightinggame {

    struct FGVertex {
        glm::vec3 Position;
        glm::vec3 Normal;
        glm::vec2 TexCoords;
        glm::vec3 Tangent;
        glm::vec3 Bitangent;
    };

    class FGMesh {
    public:
        // mesh data
        std::vector<FGVertex>         vertices;
        std::vector<unsigned int>   indices;
        std::vector<Ref<texture2D>> textures;
        std::string                 name;

        FGMesh
        (
            std::vector<FGVertex> vertices,
            std::vector<unsigned int> indices,
            std::vector< Ref<texture2D>> textures,
            std::string name
        );
        void draw(Shader& shader);
        void setupMesh();

    private:
        //  render data
        unsigned int VAO, VBO, EBO;
    };

    class FGModel
    {
    public:
        FGModel(std::string path) //e.g. 'C:/model.obj'
        {
            loadModel(path);
        }
        void Draw(Shader& shader);
        std::vector<Ref<texture2D>> get_textures() { return textures_loaded; }

    private:

        void loadModel(std::string path);
        void processNode(aiNode* node, const aiScene* scene);
        FGMesh processMesh(aiMesh* mesh, const aiScene* scene);

        std::vector<Ref<texture2D>> loadMaterialTextures
        (
            aiMaterial* mat,
            aiTextureType type,
            std::string typeName
        );

    private:
        std::vector<FGMesh> meshes;
        std::vector<Ref<texture2D>> textures_loaded;
        std::string directory;
    };

}
