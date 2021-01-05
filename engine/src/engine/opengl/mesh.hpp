#pragma once

#include <functional>
#include <vector>

#include <glm/glm.hpp>

namespace fightingengine {

enum class TOPOLOGY
{
  POINTS,
  LINES,
  LINE_STRIP,
  TRIANGLES,
  TRIANGLE_STRIP,
  TRIANGLE_FAN,
};

class Mesh
{
public:
  unsigned int vao = 0;
  unsigned int vbo = 0;
  unsigned int ebo = 0;

public:
  std::vector<glm::vec3> Positions;
  std::vector<glm::vec2> UV;
  std::vector<glm::vec3> Normals;
  std::vector<glm::vec3> Tangents;
  std::vector<glm::vec3> Bitangents;

  TOPOLOGY topology = TOPOLOGY::TRIANGLES;
  std::vector<unsigned int> Indices;

  Mesh() = default;
  Mesh(std::vector<glm::vec3> positions, std::vector<unsigned int> indices);
  Mesh(std::vector<glm::vec3> positions, std::vector<glm::vec2> uv, std::vector<unsigned int> indices);
  Mesh(std::vector<glm::vec3> positions,
       std::vector<glm::vec2> uv,
       std::vector<glm::vec3> normals,
       std::vector<unsigned int> indices);
  Mesh(std::vector<glm::vec3> positions,
       std::vector<glm::vec2> uv,
       std::vector<glm::vec3> normals,
       std::vector<glm::vec3> tangents,
       std::vector<glm::vec3> bitangents,
       std::vector<unsigned int> indices);

  void SetPositions(std::vector<glm::vec3> positions);
  void SetUVs(std::vector<glm::vec2> uv);
  void SetNormals(std::vector<glm::vec3> normals);
  void SetTangents(std::vector<glm::vec3> tangents, std::vector<glm::vec3> bitangents);

  // commits all buffers and attributes to the GPU driver
  void Finalize(bool interleaved = true);

  // binds the VAO, draws the triangles
  // void draw(Shader& shader);
};

}
