
// header
#include "engine/opengl/model.hpp"

// other project libs
#include "assimp/Importer.hpp"
#include "assimp/postprocess.h"
#include "assimp/scene.h"
#include <stb_image.h>

namespace fightingengine {

Texture2D
load_texture_for_model(std::string full_path)
{
  StbLoadedTexture stb_tex = load_texture(full_path);

  Texture2D tex;
  tex.generate(stb_tex);

  return tex;
}

Model::Model(const std::string& full_path, bool enable_textures)
{
  this->enabled_textures = enable_textures;
  this->directory = full_path.substr(0, full_path.find_last_of('/'));

  load_model(full_path);
}

void
Model::draw(Shader& s) const
{
  for (int i = 0; i < meshes.size(); i++) {
    Mesh m = meshes[i];
    render_mesh(m);
  }
};

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

  process_node(scene->mRootNode, scene);
}

void
Model::process_node(aiNode* node, const aiScene* scene)
{
  // process all the node's meshes (if any)
  for (unsigned int i = 0; i < node->mNumMeshes; i++) {
    aiMesh* mesh = scene->mMeshes[node->mMeshes[i]];
    meshes.push_back(process_mesh(mesh, scene));
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
    } else {
      vertex.tex_coords = glm::vec2(0.0f, 0.0f);
    }
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
  // process material
  if (this->enabled_textures && mesh->mMaterialIndex >= 0) {
    aiMaterial* material = scene->mMaterials[mesh->mMaterialIndex];

    std::vector<Texture2D> diffuseMaps = load_material_textures(material, aiTextureType_DIFFUSE, TextureType::DIFFUSE);
    textures.insert(textures.end(), diffuseMaps.begin(), diffuseMaps.end());

    std::vector<Texture2D> specularMaps =
      load_material_textures(material, aiTextureType_SPECULAR, TextureType::SPECULAR);
    textures.insert(textures.end(), specularMaps.begin(), specularMaps.end());
  }

  std::cout << "Loaded mesh with vertices: " << vertices.size() << ", indices: " << indices.size() << std::endl;

  Mesh m;
  m.verts = vertices;
  m.indices = indices;
  m.topology = TOPOLOGY::TRIANGLES;
  m.setup_mesh();

  return m;
}

std::vector<Texture2D>
Model::load_material_textures(aiMaterial* mat, aiTextureType type, TextureType type2)
{
  std::vector<Texture2D> textures;

  for (unsigned int i = 0; i < mat->GetTextureCount(type); i++) {

    aiString str;
    mat->GetTexture(type, i, &str);
    bool skip = false;
    for (unsigned int j = 0; j < loaded_textures.size(); j++) {
      if (std::strcmp(loaded_textures[j].path.data(), str.C_Str()) == 0) {
        textures.push_back(loaded_textures[j]);
        skip = true;
        break;
      }
    }

    if (!skip) { // if texture hasn't been loaded already, load it

      std::string full_path = directory + str.C_Str();
      std::cout << "loading new texture for model from: " << full_path << std::endl;

      Texture2D texture = load_texture_for_model(full_path);
      texture.type = type2;
      texture.path = str.C_Str();
      textures.push_back(texture);

      loaded_textures.push_back(texture); // add to loaded textures
    }

    return textures;
  }
}

} // namespace fightingengine