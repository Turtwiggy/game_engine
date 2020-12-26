#pragma once

// c++ standard libs
#include <memory>
#include <string>
#include <vector>

// other lib headers
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <glm/glm.hpp>

// your lib headers
#include "engine/mesh/mesh.hpp"
#include "engine/scene/scene_node.hpp"

namespace fightingengine {

class MeshLoader
{
public:
  // static void       clear();
  static std::vector<Mesh> load_mesh(std::string path, bool default_material = true);

private:
  static std::vector<Mesh> process_node(aiNode* aNode,
                                        const aiScene* aScene,
                                        std::string directory,
                                        bool set_default_material);
  static Mesh parse_mesh(aiMesh* aMesh,
                         const aiScene* aScene,
                         glm::vec3& out_Min,
                         glm::vec3& out_Max);

  static std::string process_path(aiString* path, std::string directory);
};

} // namespace fightingengine