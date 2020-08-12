#pragma once

#include "3d/geometry/model.hpp"

#include <string>
#include <vector>

namespace fightingengine {

    class ModelManager
    {
    public:
        std::shared_ptr<Model> add_model(const Model& model)
        {
            models.push_back(std::make_shared<Model>(model));
            return models.back();
        }

        const std::shared_ptr<Model> get_model(const std::string& name)
        {
            for (const auto& val : models)
            {
                if (val->get_name() == name)
                {
                    return val;
                }
            }
        }

        std::shared_ptr<Model> load_model(std::string path, std::string unique_name)
        {
            std::cout << "loading model from: " << path << std::endl;

            Assimp::Importer import;
            const aiScene* scene = import.ReadFile(path, aiProcess_Triangulate | aiProcess_FlipUVs);

            if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode)
            {
                std::cout << "ERROR::ASSIMP::" << import.GetErrorString() << std::endl;
                return nullptr;
            }

            std::string directory = path.substr(0, path.find_last_of('/'));

            Model model = Model(scene, directory, unique_name);

            return add_model(model);
        }

    private:
        std::vector<std::shared_ptr<Model>> models;
    };

}
