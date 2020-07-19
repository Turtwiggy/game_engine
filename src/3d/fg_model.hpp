#pragma once

#include <3d/fg_mesh.hpp>

#include <string>
#include <vector>

namespace fightinggame {

    class FGModel
    {
    public:
        FGModel() = default;
        FGModel(const aiScene* raw_model, std::string directory, std::string name)
            : directory(directory)
            , name(name)
        {
            process_node(raw_model->mRootNode, raw_model);
        }

        std::string get_name() { return name; }

        void draw(Shader& shader, uint32_t& draw_calls);

        std::vector<Ref<Texture2D>> get_textures() { return textures_loaded; }

    private:

        void process_node(aiNode* node, const aiScene* scene);
        FGMesh process_mesh(aiMesh* mesh, const aiScene* scene);

        std::vector<Ref<Texture2D>> load_material_textures
        (
            aiMaterial* mat,
            aiTextureType type,
            std::string typeName
        );

    private:
        std::vector<FGMesh> meshes;
        std::vector<Ref<Texture2D>> textures_loaded;
        std::string directory;
        std::string name;
    };
}
