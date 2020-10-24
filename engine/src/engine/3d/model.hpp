#pragma once

#include <memory>
#include <string>
#include <vector>

#include <stb_image.h>
#include <assimp/Importer.hpp>
#include <assimp/scene.h>

#include "engine/3d/mesh.hpp"
#include "engine/graphics/colour.hpp"

namespace fightingengine {

    class Model
    {
    public:

        Model() = default;

        // std::vector<Triangle> get_all_triangles_in_meshes();

        void init(const aiScene* raw_model, const std::string& name);
        void draw(Shader& shader, uint32_t& draw_calls, int texture = -1);

    private:

        void process_node(aiNode* node, const aiScene* scene);
        Mesh process_mesh(aiMesh* mesh, const aiScene* scene);

        // std::vector<Texture2D> load_material_textures
        // (
        //     aiMaterial* mat,
        //     aiTextureType type,
        //     std::string typeName
        // );

    private:
        std::vector<Mesh> meshes;
        //std::vector<Texture2D> textures_loaded;
        //std::string directory;
    };
}
