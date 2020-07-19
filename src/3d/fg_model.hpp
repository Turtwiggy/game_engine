#pragma once

#include <3d/fg_mesh.hpp>

#include <string>
#include <vector>

namespace fightinggame {

    class FGModel
    {
    public:
        FGModel(std::string path) //e.g. 'C:/model.obj'
        {
            load_model(path);
        }
        void draw(Shader& shader, uint32_t& draw_calls);
        std::vector<Ref<Texture2D>> get_textures() { return textures_loaded; }

    private:

        void load_model(std::string path);
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
    };
}
