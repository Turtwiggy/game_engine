#pragma once

#include <functional>
#include <vector>

#include <glm/glm.hpp>

#include "engine/opengl/shader.hpp"
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
  unsigned int vao = 0;
  unsigned int vbo = 0;
  unsigned int ebo = 0;

public:
  std::vector<Vertex> verts;
  std::vector<unsigned int> indices;

  TOPOLOGY topology = TOPOLOGY::TRIANGLES;

  // commits all buffers and attributes to the GPU driver
  void setup_mesh(bool interleaved = true);
};

void
render_mesh(std::shared_ptr<Mesh> mesh);

void
render_mesh(Mesh& mesh);

namespace primitives {

// class Plane
// {
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
