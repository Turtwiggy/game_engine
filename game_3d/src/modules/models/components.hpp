#pragma once

#include <string>
#include <vector>

#include "glm/glm.hpp"

namespace game2d {

struct BoneInfo
{
  std::string name;
  int id = -1;
  glm::mat4 offset = glm::mat4(1.0);
};

constexpr int MAX_BONE_INFLUENCE = 4;
struct Vertex
{
  glm::vec3 position;
  glm::vec3 normal;
  glm::vec2 uv;
  glm::vec4 colour = glm::vec4(232 / 255.0f, 97 / 255.0f, 160 / 255.0f, 1.0f);
  int bone_ids[MAX_BONE_INFLUENCE];
  float weights[MAX_BONE_INFLUENCE];
};

struct Mesh
{
  std::vector<Vertex> vertices;
  std::vector<unsigned int> indices;
  // std::vector<std::string> textures;

  unsigned int vao = 0;
  unsigned int vbo = 0;
  unsigned int ebo = 0;
};

struct Model
{
  std::string path;
  std::vector<Mesh> meshes;
  std::vector<BoneInfo> bone_info;
};

struct SINGLE_ModelsComponent
{
  Model low_poly_car{ "assets/models/low_poly_car.obj" };
};

// hack: remove this component
struct CarComponent
{
  bool placeholder = true;
};

} // namespace game2d