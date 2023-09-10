#include "helpers.hpp"

#include <GL/glew.h>
#include <assimp/Importer.hpp>
#include <assimp/postprocess.h>
#include <assimp/scene.h>

#include <algorithm>
#include <iostream>
#include <vector>

namespace game2d {

void
set_vertex_bonedata_to_default(Vertex& v)
{
  for (int i = 0; i < MAX_BONE_INFLUENCE; i++) {
    v.bone_ids[i] = -1;
    v.weights[i] = 0.0f;
  }
};

void
set_vertex_bonedata(Vertex& v, int bone_id, float weight)
{
  for (int i = 0; i < MAX_BONE_INFLUENCE; i++) {
    if (v.bone_ids[i] < 0) {
      v.bone_ids[i] = bone_id;
      v.weights[i] = weight;
      break;
    }
  }
};

void
extract_bone_weight_for_vertices(Model& model, std::vector<Vertex>& vertices, aiMesh* mesh, const aiScene* scene)
{
  auto& bone_info = model.bone_info;

  for (int i = 0; i < mesh->mNumBones; i++) {
    int bone_id = -1;
    aiBone* bone = mesh->mBones[i];
    std::string bone_name = bone->mName.C_Str();

    auto exiting_bone =
      std::find_if(bone_info.begin(), bone_info.end(), [&bone_name](const BoneInfo& b) { return b.name == bone_name; });

    if (exiting_bone == bone_info.end()) {
      BoneInfo new_bone;
      new_bone.id = bone_info.size();
      bone_id = new_bone.id;

      // convert from aiMatrix4x4 to glm::mat4
      // clang-format off
      auto& from = bone->mOffsetMatrix;
      auto& to = new_bone.offset;
      //the a,b,c,d in assimp is the row ; the 1,2,3,4 is the column
      to[0][0] = from.a1; to[1][0] = from.a2; to[2][0] = from.a3; to[3][0] = from.a4;
      to[0][1] = from.b1; to[1][1] = from.b2; to[2][1] = from.b3; to[3][1] = from.b4;
      to[0][2] = from.c1; to[1][2] = from.c2; to[2][2] = from.c3; to[3][2] = from.c4;
      to[0][3] = from.d1; to[1][3] = from.d2; to[2][3] = from.d3; to[3][3] = from.d4;
      // clang-format on

      bone_info.push_back(new_bone);
    } else
      bone_id = (*exiting_bone).id;

    for (int i = 0; i < bone->mNumWeights; i++) {
      int vertex_id = bone->mWeights[i].mVertexId;
      int weight = bone->mWeights[i].mWeight;
      set_vertex_bonedata(vertices[vertex_id], bone_id, weight);
    }
  }
}

Mesh
process_mesh(Model& model, aiMesh* mesh, const aiScene* scene)
{
  std::vector<Vertex> vertices;
  std::vector<unsigned int> indices;
  // std::vector<std::string> textures;

  // verts
  for (int i = 0; i < mesh->mNumVertices; i++) {
    Vertex v;
    set_vertex_bonedata_to_default(v);

    v.position.x = mesh->mVertices[i].x;
    v.position.y = mesh->mVertices[i].y;
    v.position.z = mesh->mVertices[i].z;

    if (mesh->HasNormals()) {
      v.normal.x = mesh->mNormals[i].x;
      v.normal.y = mesh->mNormals[i].y;
      v.normal.z = mesh->mNormals[i].z;
    }

    if (mesh->mTextureCoords[0]) // does the mesh contain texture coordinates?
    {
      v.uv.x = mesh->mTextureCoords[0][i].x;
      v.uv.y = mesh->mTextureCoords[0][i].y;
    } else {
      v.uv = glm::vec2(0.0f, 0.0f);
    }

    if (mesh->mMaterialIndex >= 0) {
      aiMaterial* mat = scene->mMaterials[mesh->mMaterialIndex];
      aiColor3D color(0.f, 0.f, 0.f);
      mat->Get(AI_MATKEY_COLOR_DIFFUSE, color);
      v.colour.r = color.r;
      v.colour.g = color.g;
      v.colour.b = color.b;
    }

    vertices.push_back(v);
  }

  // indices
  for (int i = 0; i < mesh->mNumFaces; i++) {
    aiFace face = mesh->mFaces[i];
    for (unsigned int j = 0; j < face.mNumIndices; j++)
      indices.push_back(face.mIndices[j]);
  }

  // Materials
  if (mesh->mMaterialIndex >= 0) {
    aiMaterial* mat = scene->mMaterials[mesh->mMaterialIndex];

    // aiColor3D color(0.f, 0.f, 0.f);
    // mat->Get(AI_MATKEY_COLOR_DIFFUSE, color);
    // v.colour.r = color.r;
    // v.colour.g = color.g;
    // v.colour.b = color.b;

    // const auto get_texture_paths = [&mat](const aiTextureType& type) -> std::vector<std::string> {
    //   std::vector<std::string> result;
    //   for (int i = 0; i < mat->GetTextureCount(type); i++) {
    //     aiString str;
    //     mat->GetTexture(type, i, &str);
    //     result.push_back(str.C_Str());
    //   }
    //   return result;
    // };
    // auto base = get_texture_paths(aiTextureType_BASE_COLOR);
    // textures = textures.insert(textures.end(), base.begin(), base.end());
    // textures = get_texture_paths(aiTextureType_DIFFUSE);
    // textures = get_texture_paths(aiTextureType_SPECULAR);
  }

  // Animations
  // Each aiBone contains the information like how much influence
  // this bone will have on a set of vertices on the mesh.
  extract_bone_weight_for_vertices(model, vertices, mesh, scene);

  return std::move(Mesh(std::move(vertices), std::move(indices)));
}

void
process_node(Model& model, aiNode* node, const aiScene* scene)
{
  for (int i = 0; i < node->mNumMeshes; i++) {
    aiMesh* mesh = scene->mMeshes[node->mMeshes[i]];
    model.meshes.push_back(std::move(process_mesh(model, mesh, scene)));
  }
  for (int i = 0; i < node->mNumChildren; i++)
    process_node(model, node->mChildren[i], scene);
};

void
load_models(SINGLE_ModelsComponent& models)
{
  Assimp::Importer importer;

  // choose models
  auto& model = models.low_poly_car;

  // load
  const auto flags = aiProcess_Triangulate | aiProcess_FlipUVs;
  const aiScene* scene = importer.ReadFile(model.path, flags);
  if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode) {
    std::cerr << "ERROR::ASSIMP::" << importer.GetErrorString() << std::endl;
    return;
  }

  // process
  aiNode* root = scene->mRootNode;
  process_node(model, root, scene);

  // opengl
  for (auto& mesh : model.meshes) {
    auto& vertices = mesh.vertices;
    auto& indices = mesh.indices;

    // generate some buffers
    glGenVertexArrays(1, &mesh.vao);
    glGenBuffers(1, &mesh.vbo);
    glGenBuffers(1, &mesh.ebo);
    glBindVertexArray(mesh.vao);

    // bind vertices
    glBindBuffer(GL_ARRAY_BUFFER, mesh.vbo);
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(Vertex), &vertices[0], GL_STATIC_DRAW);

    // bind indices
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mesh.ebo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), &indices[0], GL_STATIC_DRAW);

    // set layout for shader
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)0);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, normal));
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, uv));
    glEnableVertexAttribArray(3);
    glVertexAttribPointer(3, 4, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, colour));
    glEnableVertexAttribArray(4);
    glVertexAttribIPointer(4, 4, GL_INT, sizeof(Vertex), (void*)offsetof(Vertex, bone_ids));
    glEnableVertexAttribArray(5);
    glVertexAttribPointer(5, 4, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, weights));
    glBindVertexArray(0);
  }

  std::cout << "loaded car model..." << std::endl;
};

// if want textures, bind textures to shader/texture-unit
void
draw_model(const Model& m)
{
  for (const auto& m : m.meshes) {
    glBindVertexArray(m.vao);
    glDrawElements(GL_TRIANGLES, static_cast<unsigned int>(m.indices.size()), GL_UNSIGNED_INT, 0);
    glBindVertexArray(0);
  }
}

} // namespace game2d