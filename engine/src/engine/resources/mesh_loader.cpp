
// header
#include "engine/resources/mesh_loader.hpp"

// c++ lib headers
#include <memory>

// other lib headers
#include <assimp/postprocess.h>
#include <spdlog/spdlog.h>

// your project headers
#include "engine/mesh/mesh.hpp"

namespace fightingengine {

std::vector<Mesh>
MeshLoader::process_node(aiNode* aNode,
                         const aiScene* aScene,
                         std::string directory,
                         bool set_default_material)
{
  std::vector<Mesh> meshes;

  // process all of the node's meshes (if any)
  for (unsigned int i = 0; i < aNode->mNumMeshes; ++i) {
    glm::vec3 boxMin, boxMax;
    aiMesh* assimpMesh = aScene->mMeshes[aNode->mMeshes[i]];
    Mesh mesh = parse_mesh(assimpMesh, aScene, boxMin, boxMax);
    meshes.push_back(mesh);

    // TODO(Turtwiggy) process materials

    aiMaterial* assimpMat = aScene->mMaterials[assimpMesh->mMaterialIndex];

    // // 1. diffuse maps
    // std::vector<Texture2D> diffuseMaps = load_material_textures(material,
    // aiTextureType_DIFFUSE, "texture_diffuse");
    // textures.insert(textures.end(), diffuseMaps.begin(), diffuseMaps.end());
    // // 2. specular maps
    // std::vector<Texture2D> specularMaps = load_material_textures(material,
    // aiTextureType_SPECULAR, "texture_specular");
    // textures.insert(textures.end(), specularMaps.begin(),
    // specularMaps.end());
    // // 3. normal maps
    // std::vector<Texture2D> normalMaps = load_material_textures(material,
    // aiTextureType_HEIGHT, "texture_normal"); textures.insert(textures.end(),
    // normalMaps.begin(), normalMaps.end());
    // // 4. height maps
    // std::vector<Texture2D> heightMaps = load_material_textures(material,
    // aiTextureType_AMBIENT, "texture_height"); textures.insert(textures.end(),
    // heightMaps.begin(), heightMaps.end());

    ////Give each mesh its colour
    // aiColor3D mat_colour(1.f, 1.f, 1.f);
    // material->Get(AI_MATKEY_COLOR_DIFFUSE, mat_colour);

    // TODO(Turtwiggy) handle the bounding boxes (boxMin, boxMax);
  }

  // also recursively parse this node's children
  for (unsigned int i = 0; i < aNode->mNumChildren; ++i) {
    // TODO(Turtwiggy)
    // sprocess_node(aNode->mChildren[i], aScene, directory,
    // set_default_material);
  }

  return meshes;
}

Mesh
MeshLoader::parse_mesh(aiMesh* aMesh,
                       const aiScene* aScene,
                       glm::vec3& out_Min,
                       glm::vec3& out_Max)
{
  std::vector<glm::vec3> positions;
  std::vector<glm::vec2> uv;
  std::vector<glm::vec3> normals;
  std::vector<glm::vec3> tangents;
  std::vector<glm::vec3> bitangents;
  std::vector<unsigned int> indices;

  positions.resize(aMesh->mNumVertices);
  normals.resize(aMesh->mNumVertices);
  if (aMesh->mNumUVComponents > 0) {
    uv.resize(aMesh->mNumVertices);
    tangents.resize(aMesh->mNumVertices);
    bitangents.resize(aMesh->mNumVertices);
  }
  // we assume a constant of 3 vertex indices per face as we always triangulate
  // in Assimp's post-processing step; otherwise you'll want transform this to a
  // more  flexible scheme.
  indices.resize(aMesh->mNumFaces * 3);

  // store min/max point in local coordinates for calculating approximate
  // bounding box.
  glm::vec3 pMin(99999.0);
  glm::vec3 pMax(-99999.0);

  for (unsigned int i = 0; i < aMesh->mNumVertices; ++i) {
    positions[i] =
      glm::vec3(aMesh->mVertices[i].x, aMesh->mVertices[i].y, aMesh->mVertices[i].z);
    normals[i] =
      glm::vec3(aMesh->mNormals[i].x, aMesh->mNormals[i].y, aMesh->mNormals[i].z);
    if (aMesh->mTextureCoords[0]) {
      uv[i] = glm::vec2(aMesh->mTextureCoords[0][i].x, aMesh->mTextureCoords[0][i].y);
    }
    if (aMesh->mTangents) {
      tangents[i] =
        glm::vec3(aMesh->mTangents[i].x, aMesh->mTangents[i].y, aMesh->mTangents[i].z);
      bitangents[i] = glm::vec3(
        aMesh->mBitangents[i].x, aMesh->mBitangents[i].y, aMesh->mBitangents[i].z);
    }
    if (positions[i].x < pMin.x)
      pMin.x = positions[i].x;
    if (positions[i].y < pMin.y)
      pMin.y = positions[i].y;
    if (positions[i].z < pMin.z)
      pMin.z = positions[i].z;
    if (positions[i].x > pMax.x)
      pMax.x = positions[i].x;
    if (positions[i].y > pMax.y)
      pMax.y = positions[i].y;
    if (positions[i].z > pMax.z)
      pMax.z = positions[i].z;
  }
  for (unsigned int f = 0; f < aMesh->mNumFaces; ++f) {
    // we know we're always working with triangles due to TRIANGULATE option.
    for (unsigned int i = 0; i < 3; ++i) {
      indices[f * 3 + i] = aMesh->mFaces[f].mIndices[i];
    }
  }

  Mesh mesh;
  mesh.Positions = positions;
  mesh.UV = uv;
  mesh.Normals = normals;
  mesh.Tangents = tangents;
  mesh.Bitangents = bitangents;
  mesh.Indices = indices;
  mesh.topology = TOPOLOGY::TRIANGLES;
  mesh.Finalize(true);

  out_Min.x = pMin.x;
  out_Min.y = pMin.y;
  out_Min.z = pMin.z;
  out_Max.x = pMax.x;
  out_Max.y = pMax.y;
  out_Max.z = pMax.z;

  // store newly generated mesh in globally stored mesh store for memory
  // de-allocation when a clean is required.
  // MeshLoader::meshStore.push_back(mesh);

  return mesh;
}

std::vector<Mesh>
MeshLoader::load_mesh(std::string path, bool set_default_material)
{
  Assimp::Importer import;
  const aiScene* scene =
    import.ReadFile(path, aiProcess_Triangulate | aiProcess_FlipUVs);

  if (!scene || scene->mFlags == AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode) {
    spdlog::error("Assimp failed to load model at path:  '{}'", path);
    return {};
  }

  std::string directory = path.substr(0, path.find_last_of("/"));

  return process_node(scene->mRootNode, scene, directory, set_default_material);
}

std::string
MeshLoader::process_path(aiString* aPath, std::string directory)
{
  std::string path = std::string(aPath->C_Str());
  // parse path directly if path contains "/" indicating it is an absolute path;
  // otherwise parse as relative.
  if (path.find(":/") == std::string::npos || path.find(":\\") == std::string::npos)
    path = directory + "/" + path;
  return path;
}

// // checks all material textures of a given type and loads the textures if
// they're not loaded yet. the required info is returned as a Texture struct.
// std::vector<Texture2D> Model::load_material_textures(aiMaterial *mat,
// aiTextureType type, std::string typeName)
// {
//     std::vector<Texture2D> textures;
//     for (unsigned int i = 0; i < mat->GetTextureCount(type); i++)
//     {
//         aiString str;
//         mat->GetTexture(type, i, &str);
//         std::string full_path = directory + '/' + str.C_Str();

//         //check cached textures
//         for (unsigned int j = 0; j < textures_loaded.size(); j++)
//         {
//             if (textures_loaded[j].path.data() != full_path)
//             {
//                 textures.push_back(textures_loaded[j]);
//                 continue;
//             }
//         }

//         printf("Texture not loaded, path: %s name: %s \n", full_path.c_str(),
//         str.C_Str()); Texture2D texture =
//         ResourceManager::load_texture(full_path.c_str(), false, str.C_Str());
//         texture.type = typeName;
//         texture.path = full_path;
//         textures.push_back(texture);

//         textures_loaded.push_back(texture); // store it as texture loaded for
//         entire model, to ensure we won't unnecesery load duplicate textures.
//     }

//     return textures;
// }

} // namespace fightingengine