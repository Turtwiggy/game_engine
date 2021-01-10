#pragma once

#include <functional>
#include <vector>

#include <glm/glm.hpp>

#include "engine/opengl/shader.hpp"
#include "engine/opengl/texture.hpp"
#include "engine/opengl/vertex.hpp"

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
  std::vector<Vertex> verts;
  std::vector<unsigned int> indices;
  std::vector<Texture2D> textures;

public:
  Mesh(std::vector<Vertex> vertices, std::vector<unsigned int> indices, std::vector<Texture2D> textures);

  void draw(Shader& shader);

private:
  unsigned int vao, vbo, ebo = 0;

  // commits all buffers and attributes to the GPU driver
  void setup_mesh();
};
namespace primitives {

// class Plane
// { o
// public:
//   Mesh mesh; // set in constructor

//   Plane();
// };

// class Cube
// {
// public:
//   Mesh mesh; // set in constructor

//   Cube();
// };

} // namespace primitives

}
