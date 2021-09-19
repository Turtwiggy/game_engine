
// header
#include "engine/opengl/model.hpp"

// other project libs
#include <stb_image.h>

// standard lib headers
#include <iostream>
#include <string>

namespace engine {

Model::Model(const std::string& full_path)
{
  load_model(full_path);
}

void
Model::draw(Shader& shader)
{
  for (int i = 0; i < this->meshes.size(); i++)
    this->meshes[i].draw(shader);
}

void
Model::load_model(const std::string& path)
{
  std::cout << "loading model: " << path << std::endl;

  Assimp::Importer importer;
  const aiScene* scene = importer.ReadFile(path, aiProcess_Triangulate | aiProcess_FlipUVs);

  if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode) {
    const auto& err = importer.GetErrorString();
    std::cerr << "Failed to load scene: " << err << std::endl;
    return;
  }

  this->directory = path.substr(0, path.find_last_of('/'));
  std::cout << "loading model from directory: " << directory << std::endl;

  process_node(scene->mRootNode, scene);
}

void
Model::process_node(aiNode* node, const aiScene* scene)
{
  // process all the node's meshes (if any)
  for (unsigned int i = 0; i < node->mNumMeshes; i++) {
    aiMesh* mesh = scene->mMeshes[node->mMeshes[i]];
    Mesh m = process_mesh(mesh, scene);
    this->meshes.push_back(m);
  }
  // then do the same for each of its children
  for (unsigned int i = 0; i < node->mNumChildren; i++) {
    process_node(node->mChildren[i], scene);
  }
}

// struct VertexWithBoneData
// {
//   Vertex v; // pos, normal, tex_coords

//   const int num_bones_per_vertex = 4;
//   uint ids[num_bones_per_vertex];
//   float weights[num_bones_per_vertex];
// };

Mesh
Model::process_mesh(aiMesh* mesh, const aiScene* scene)
{
  std::vector<Vertex> vertices;
  std::vector<unsigned int> indices;

  //
  // TODO finish animations see: player_model.cc
  // http://ogldev.atspace.co.uk/www/tutorial38/tutorial38.html
  //

  // each mesh contains aiBone[]
  // each mesh contains aiVector3D vertices[]
  // each aiBone contasn name, aiVertexWeight[], offsetMatrix
  // each aiVertexWeight[] contains vertexID, weight

  // each scene contains aiNodes
  // each scene contains aiAnimation
  // each aiAnimation contains duration, and ticker per  second
  // each aiAnimation contains aiNodeAnim

  // SEE: http://ogldev.atspace.co.uk/www/tutorial38/tutorial38.html
  // The structure above contains everything we need at the vertex level. By default, we have enough storage for four
  // bones (ID and weight per bone). VertexBoneData was structured like that to make it simple to pass it on to the
  // shader. We already got position, texture coordinates and normal bound at locations 0, 1 and 2, respectively.

  // SEE: player_model.cc line 335

  aiMatrix4x4 inverse_transform = scene->mRootNode->mTransformation.Inverse();

  // clang-format off
  
  std::cout << " mesh " << mesh->mName.C_Str()
            << "\n (bones): " << std::to_string(mesh->mNumBones)
            << "\n (animations): " << std::to_string(scene->mNumAnimations)
            << std::endl;

  if(scene->mNumAnimations > 0){
    float anim_ticks_per_second = scene->mAnimations[0]->mTicksPerSecond;
    float anim_time_in_seconds = 2.0f;
    float anim_time_in_ticks = anim_time_in_seconds * anim_ticks_per_second;
  }

  // clang-format on

  for (int i = 0; i < mesh->mNumBones; i++) {
    aiBone* bone = mesh->mBones[i];
    const std::string bone_name = bone->mName.C_Str();
    std::cout << "found a bone!: " << bone_name << std::endl;
  }

  for (unsigned int i = 0; i < mesh->mNumVertices; i++) {
    Vertex vertex;
    { // Position
      glm::vec3 vector;
      vector.x = mesh->mVertices[i].x;
      vector.y = mesh->mVertices[i].y;
      vector.z = mesh->mVertices[i].z;
      vertex.position = vector;
    }
    { // Normal
      glm::vec3 vector;
      vector.x = mesh->mNormals[i].x;
      vector.y = mesh->mNormals[i].y;
      vector.z = mesh->mNormals[i].z;
      vertex.normal = vector;
    }
    if (mesh->mTextureCoords[0]) {
      glm::vec2 vec;
      vec.x = mesh->mTextureCoords[0][i].x;
      vec.y = mesh->mTextureCoords[0][i].y;
      vertex.tex_coords = vec;
    } else
      vertex.tex_coords = glm::vec2(0.0f, 0.0f);

    vertices.push_back(vertex);
  }

  // process indices
  for (unsigned int i = 0; i < mesh->mNumFaces; i++) {
    aiFace face = mesh->mFaces[i];
    for (unsigned int j = 0; j < face.mNumIndices; j++) {
      const unsigned int vertexIndex = face.mIndices[j];
      if (vertexIndex >= vertices.size()) {
        std::cerr << "Unexpected element index, " << vertexIndex << " vs vertex count: " << vertices.size()
                  << std::endl;
        exit(1);
      }
      indices.push_back(vertexIndex);
    }
  }
  std::cout << "Loaded mesh with vertices: " << vertices.size() << ", indices: " << indices.size() << std::endl;

  // process materials
  aiMaterial* material = scene->mMaterials[mesh->mMaterialIndex];
  // we assume a convention for sampler names in the shaders. Each diffuse texture should be named
  // as 'texture_diffuseN' where N is a sequential number ranging from 1 to MAX_SAMPLER_NUMBER.
  // Same applies to other texture as the following list summarizes:
  // diffuse: texture_diffuseN
  // specular: texture_specularN
  // normal: texture_normalN

  return Mesh(vertices, indices);
}

} // namespace engine