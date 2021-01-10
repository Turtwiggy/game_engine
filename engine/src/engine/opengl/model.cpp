
// header
#include "engine/opengl/model.hpp"

// other project libs
#include "assimp/Importer.hpp"
#include "assimp/postprocess.h"
#include "assimp/scene.h"
#include <stb_image.h>

namespace fightingengine {

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
    std::cerr << "Failed to load scene: " << importer.GetErrorString() << std::endl;
    return;
  }

  this->directory = path.substr(0, path.find_last_of('/'));
  std::cout << "loading model: " << directory << std::endl;

  process_node(scene->mRootNode, scene);
}

void
Model::process_node(aiNode* node, const aiScene* scene)
{
  // process all the node's meshes (if any)
  for (unsigned int i = 0; i < node->mNumMeshes; i++) {
    aiMesh* mesh = scene->mMeshes[node->mMeshes[i]];
    Mesh m = process_mesh(mesh, scene);
    meshes.push_back(m);
  }
  // then do the same for each of its children
  for (unsigned int i = 0; i < node->mNumChildren; i++) {
    process_node(node->mChildren[i], scene);
  }
}

Mesh
Model::process_mesh(aiMesh* mesh, const aiScene* scene)
{
  std::vector<Vertex> vertices;
  std::vector<unsigned int> indices;
  std::vector<Texture2D> textures;

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
  // process materials
  aiMaterial* material = scene->mMaterials[mesh->mMaterialIndex];
  // we assume a convention for sampler names in the shaders. Each diffuse texture should be named
  // as 'texture_diffuseN' where N is a sequential number ranging from 1 to MAX_SAMPLER_NUMBER.
  // Same applies to other texture as the following list summarizes:
  // diffuse: texture_diffuseN
  // specular: texture_specularN
  // normal: texture_normalN

  // 1. diffuse maps
  std::vector<Texture2D> diffuseMaps = load_material_textures(material, aiTextureType_DIFFUSE, TextureType::DIFFUSE);
  textures.insert(textures.end(), diffuseMaps.begin(), diffuseMaps.end());
  // 2. specular maps
  std::vector<Texture2D> specularMaps = load_material_textures(material, aiTextureType_SPECULAR, TextureType::SPECULAR);
  textures.insert(textures.end(), specularMaps.begin(), specularMaps.end());
  // 3. normal maps
  std::vector<Texture2D> normalMaps = load_material_textures(material, aiTextureType_HEIGHT, TextureType::NORMAL);
  textures.insert(textures.end(), normalMaps.begin(), normalMaps.end());
  // 4. height maps
  std::vector<Texture2D> heightMaps = load_material_textures(material, aiTextureType_AMBIENT, TextureType::HEIGHT);
  textures.insert(textures.end(), heightMaps.begin(), heightMaps.end());

  std::cout << "Loaded mesh with vertices: " << vertices.size() << ", indices: " << indices.size() << std::endl;

  return Mesh(vertices, indices, textures);
}

std::vector<Texture2D>
Model::load_material_textures(aiMaterial* mat, aiTextureType type, TextureType type2)
{
  std::vector<Texture2D> textures;

  for (unsigned int i = 0; i < mat->GetTextureCount(type); i++) {
    aiString str;
    mat->GetTexture(type, i, &str);
    // check if texture was loaded before and if so, continue to next iteration: skip loading a new texture
    bool skip = false;
    for (unsigned int j = 0; j < loaded_textures.size(); j++) {
      if (std::strcmp(loaded_textures[j].path.data(), str.C_Str()) == 0) {
        textures.push_back(loaded_textures[j]);
        skip = true; // a texture with the same filepath has already been loaded, continue to next one. (optimization)
        break;
      }
    }
    if (!skip) { // if texture hasn't been loaded already, load it

      std::string full_path = this->directory + "/" + str.C_Str();
      std::cout << "loading texture from: " << full_path << std::endl;

      Texture2D texture;
      texture.load_texture_from_file(full_path);
      texture.type = type2;
      texture.path = str.C_Str();

      textures.push_back(texture);
      loaded_textures.push_back(texture);
    }
  }
  return textures;
}

} // namespace fightingengine