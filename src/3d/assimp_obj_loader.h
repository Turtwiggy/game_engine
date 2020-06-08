#pragma once

#include <graphics/shader.h>

//#include "assimp/"
#include "glm/glm.hpp"
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

#include <string>
#include <vector>

namespace fightinggame {

    struct Vertex {
        glm::vec3 Position;
        glm::vec3 Normal;
        glm::vec2 TexCoords;
        glm::vec3 Tangent;
        glm::vec3 Bitangent;
    };

    struct Texture {
        unsigned int id;
        std::string type;
        std::string path;  // we store the path of the texture to compare with other textures
    };

    class Mesh {
    public:
        // mesh data
        std::vector<Vertex>       vertices;
        std::vector<unsigned int> indices;
        std::vector<Texture>      textures;

        Mesh(std::vector<Vertex> vertices, std::vector<unsigned int> indices, std::vector<Texture> textures);
        void Draw(Shader& shader);
        void setupMesh();

    private:
        //  render data
        unsigned int VAO, VBO, EBO;
    };

    class Model
    {
    public:
        Model(std::string path)
        {
            loadModel(path);
        }
        void Draw(Shader& shader);
    private:
        // model data
        std::vector<Mesh> meshes;
        std::vector<Texture> textures_loaded;
        std::string directory;
        bool gammaCorrection;

        void loadModel(std::string path);
        void processNode(aiNode* node, const aiScene* scene);
        Mesh processMesh(aiMesh* mesh, const aiScene* scene);
        std::vector<Texture> loadMaterialTextures
        (
            aiMaterial* mat,
            aiTextureType type,
            std::string typeName
        );
    };

    // utility function for loading a 2D texture from file
    // ---------------------------------------------------
    unsigned int TextureFromFile(const char* path, const std::string& directory, bool gamma = true);
}
