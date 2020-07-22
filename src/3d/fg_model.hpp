#pragma once

#include <3d/fg_mesh.hpp>
#include <3d/fg_colour.hpp>

#include <stb_image.h>
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

#include <string>
#include <vector>

namespace fightinggame {


    class FGModel
    {
    public:

        FGModel(const aiScene* raw_model, std::string directory, std::string name)
            : directory(directory)
            , name(name)
        {
            process_node(raw_model->mRootNode, raw_model);
        }

        FGModel(std::shared_ptr<FGMesh> mesh, std::string name)
            : name(name)
            , directory(name)
        {
            meshes.push_back(*mesh);
        }

        std::string get_name() { return name; }

        void draw(Shader& shader, uint32_t& draw_calls);

    private:

        void process_node(aiNode* node, const aiScene* scene);
        FGMesh process_mesh(aiMesh* mesh, const aiScene* scene);

        std::vector<Texture2D> load_material_textures
        (
            aiMaterial* mat,
            aiTextureType type,
            std::string typeName
        );

    private:
        std::vector<FGMesh> meshes;
        std::vector<Texture2D> textures_loaded;
        std::string directory;
        std::string name;
        bool gammaCorrection = false;
    };
}
