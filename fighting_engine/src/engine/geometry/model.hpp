#pragma once

#include "engine/geometry/mesh.hpp"
#include "engine/renderer/colour.hpp"

#include <stb_image.h>
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

#include <memory>
#include <string>
#include <vector>

namespace fightingengine {

    class Model
    {
    public:

        Model(const aiScene* raw_model, std::string directory, std::string name)
            : directory(directory)
            , name(name)
        {
            process_node(raw_model->mRootNode, raw_model);
        }

        Model(std::shared_ptr<Mesh> mesh, std::string name)
            : name(name)
            , directory(name)
        {
            meshes.push_back(*mesh);
        }

        std::string get_name() { return name; }

        std::vector<Triangle> get_all_triangles_in_meshes();

        void draw(Shader& shader, uint32_t& draw_calls, int texture = -1);

    private:

        void process_node(aiNode* node, const aiScene* scene);
        Mesh process_mesh(aiMesh* mesh, const aiScene* scene);

        std::vector<Texture2D> load_material_textures
        (
            aiMaterial* mat,
            aiTextureType type,
            std::string typeName
        );

    private:
        std::vector<Mesh> meshes;
        std::vector<Texture2D> textures_loaded;
        std::string directory;
        std::string name;
        bool gammaCorrection = false;
    };
}
