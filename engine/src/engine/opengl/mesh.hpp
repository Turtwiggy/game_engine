#pragma once

#include "engine/opengl/shader.hpp"
#include "engine/opengl/texture.hpp"

#include <glm/glm.hpp>

#include <functional>
#include <vector>

namespace engine {

enum class TOPOLOGY
{
  POINTS,
  LINES,
  LINE_STRIP,
  TRIANGLES,
  TRIANGLE_STRIP,
  TRIANGLE_FAN,
};

struct Vertex
{
  glm::vec3 position = glm::vec3(0.0, 0.0, 0.0);
  glm::vec3 normal = glm::vec3(0.0, 0.0, 0.0);
  glm::vec2 tex_coords = glm::vec2(0.0, 0.0);
};

class Mesh
{
public:
  std::vector<Vertex> verts;
  std::vector<unsigned int> indices;

public:
  Mesh(std::vector<Vertex> vertices, std::vector<unsigned int> indices);

  void draw(Shader& shader);

private:
  unsigned int vao, vbo, ebo = 0;

  // commits all buffers and attributes to the GPU driver
  void setup_mesh();
};

}
