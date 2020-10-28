// #pragma once

// #include <memory>
// #include <string>
// #include <vector>

// #include <stb_image.h>
// #include <assimp/Importer.hpp>
// #include <assimp/scene.h>

// #include "engine/3d/mesh.hpp"

// namespace fightingengine {

//     // A model is just a series of meshes

//     class Model
//     {
//     public:

//         Model() = default;

//         void init(const aiScene* raw_model, const std::string& name);

//         [[nodiscard]] std::vector<Mesh>& get_meshes();

//     private:

//         void process_node(aiNode* node, const aiScene* scene);
//         Mesh process_mesh(aiMesh* mesh, const aiScene* scene);

//     private:
//         std::vector<Mesh> meshes;
//     };
// }
