
// header
#include "engine/opengl/primitives.hpp"

// c++ standard library
#include <vector>

// other library headers
#include <GL/glew.h>

namespace fightingengine {

void
render_mesh(std::shared_ptr<Mesh> mesh)
{
  // bind vao
  glBindVertexArray(mesh->vao);

  if (mesh->Indices.size() > 0) {
    glDrawElements(mesh->topology == TOPOLOGY::TRIANGLE_STRIP ? GL_TRIANGLE_STRIP : GL_TRIANGLES,
                   mesh->Indices.size(),
                   GL_UNSIGNED_INT,
                   0);
  } else {
    glDrawArrays(
      mesh->topology == TOPOLOGY::TRIANGLE_STRIP ? GL_TRIANGLE_STRIP : GL_TRIANGLES, 0, mesh->Positions.size());
  }

  glBindVertexArray(0);
}

void
render_mesh(Mesh& mesh)
{
  // bind vao
  glBindVertexArray(mesh.vao);

  if (mesh.Indices.size() > 0) {
    glDrawElements(mesh.topology == TOPOLOGY::TRIANGLE_STRIP ? GL_TRIANGLE_STRIP : GL_TRIANGLES,
                   mesh.Indices.size(),
                   GL_UNSIGNED_INT,
                   0);
  } else {
    glDrawArrays(
      mesh.topology == TOPOLOGY::TRIANGLE_STRIP ? GL_TRIANGLE_STRIP : GL_TRIANGLES, 0, mesh.Positions.size());
  }

  glBindVertexArray(0);
}

namespace primitives {

// ---- plane

Plane::Plane(int x, int y)
{
  Positions =
    std::vector<glm::vec3>{ glm::vec3(1.0f, 0.0f, 1.0f), glm::vec3(-1.0f, 0.0f, 1.0f),  glm::vec3(-1.0f, 0.0f, -1.0f),
                            glm::vec3(1.0f, 0.0f, 1.0f), glm::vec3(-1.0f, 0.0f, -1.0f), glm::vec3(1.0f, 0.0f, -1.0f) };

  Normals =
    std::vector<glm::vec3>{ glm::vec3(0.0f, 1.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f),

                            glm::vec3(0.0f, 1.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f) };

  UV = std::vector<glm::vec2>{
    glm::vec2(1.0f, 0.0f), glm::vec2(0.0f, 0.0f), glm::vec2(0.0f, 1.0f),

    glm::vec2(1.0f, 0.0f), glm::vec2(0.0f, 1.0f), glm::vec2(1.0f, 1.0f),
  };

  topology = TOPOLOGY::TRIANGLE_STRIP;
  Finalize();
}

// ---- cube

Cube::Cube()
{
  Positions = std::vector<glm::vec3>{
    glm::vec3(-1.0f, -1.0f, -1.0f), glm::vec3(1.0f, 1.0f, -1.0f),   glm::vec3(1.0f, -1.0f, -1.0f),
    glm::vec3(1.0f, 1.0f, -1.0f),   glm::vec3(-1.0f, -1.0f, -1.0f), glm::vec3(-1.0f, 1.0f, -1.0f),

    glm::vec3(-1.0f, -1.0f, 1.0f),  glm::vec3(1.0f, -1.0f, 1.0f),   glm::vec3(1.0f, 1.0f, 1.0f),
    glm::vec3(1.0f, 1.0f, 1.0f),    glm::vec3(-1.0f, 1.0f, 1.0f),   glm::vec3(-1.0f, -1.0f, 1.0f),

    glm::vec3(-1.0f, 1.0f, 1.0f),   glm::vec3(-1.0f, 1.0f, -1.0f),  glm::vec3(-1.0f, -1.0f, -1.0f),
    glm::vec3(-1.0f, -1.0f, -1.0f), glm::vec3(-1.0f, -1.0f, 1.0f),  glm::vec3(-1.0f, 1.0f, 1.0f),

    glm::vec3(1.0f, 1.0f, 1.0f),    glm::vec3(1.0f, -1.0f, -1.0f),  glm::vec3(1.0f, 1.0f, -1.0f),
    glm::vec3(1.0f, -1.0f, -1.0f),  glm::vec3(1.0f, 1.0f, 1.0f),    glm::vec3(1.0f, -1.0f, 1.0f),

    glm::vec3(-1.0f, -1.0f, -1.0f), glm::vec3(1.0f, -1.0f, -1.0f),  glm::vec3(1.0f, -1.0f, 1.0f),
    glm::vec3(1.0f, -1.0f, 1.0f),   glm::vec3(-1.0f, -1.0f, 1.0f),  glm::vec3(-1.0f, -1.0f, -1.0f),

    glm::vec3(-1.0f, 1.0f, -1.0f),  glm::vec3(1.0f, 1.0f, 1.0f),    glm::vec3(1.0f, 1.0f, -1.0f),
    glm::vec3(1.0f, 1.0f, 1.0f),    glm::vec3(-1.0f, 1.0f, -1.0f),  glm::vec3(-1.0f, 1.0f, 1.0f)
  };

  Normals = std::vector<glm::vec3>{

    glm::vec3(0.0f, 0.0f, -1.0f), glm::vec3(0.0f, 0.0f, -1.0f), glm::vec3(0.0f, 0.0f, -1.0f),
    glm::vec3(0.0f, 0.0f, -1.0f), glm::vec3(0.0f, 0.0f, -1.0f), glm::vec3(0.0f, 0.0f, -1.0f),

    glm::vec3(0.0f, 0.0f, 1.0f),  glm::vec3(0.0f, 0.0f, 1.0f),  glm::vec3(0.0f, 0.0f, 1.0f),
    glm::vec3(0.0f, 0.0f, 1.0f),  glm::vec3(0.0f, 0.0f, 1.0f),  glm::vec3(0.0f, 0.0f, 1.0f),

    glm::vec3(-1.0f, 0.0f, 0.0f), glm::vec3(-1.0f, 0.0f, 0.0f), glm::vec3(-1.0f, 0.0f, 0.0f),
    glm::vec3(-1.0f, 0.0f, 0.0f), glm::vec3(-1.0f, 0.0f, 0.0f), glm::vec3(-1.0f, 0.0f, 0.0f),

    glm::vec3(1.0f, 0.0f, 0.0f),  glm::vec3(1.0f, 0.0f, 0.0f),  glm::vec3(1.0f, 0.0f, 0.0f),
    glm::vec3(1.0f, 0.0f, 0.0f),  glm::vec3(1.0f, 0.0f, 0.0f),  glm::vec3(1.0f, 0.0f, 0.0f),

    glm::vec3(0.0f, -1.0f, 0.0f), glm::vec3(0.0f, -1.0f, 0.0f), glm::vec3(0.0f, -1.0f, 0.0f),
    glm::vec3(0.0f, -1.0f, 0.0f), glm::vec3(0.0f, -1.0f, 0.0f), glm::vec3(0.0f, -1.0f, 0.0f),

    glm::vec3(0.0f, 1.0f, 0.0f),  glm::vec3(0.0f, 1.0f, 0.0f),  glm::vec3(0.0f, 1.0f, 0.0f),
    glm::vec3(0.0f, 1.0f, 0.0f),  glm::vec3(0.0f, 1.0f, 0.0f),  glm::vec3(0.0f, 1.0f, 0.0f)
  };

  UV = std::vector<glm::vec2>{
    glm::vec2(0.0f, 0.0f), glm::vec2(1.0f, 1.0f), glm::vec2(1.0f, 0.0f),
    glm::vec2(1.0f, 1.0f), glm::vec2(0.0f, 0.0f), glm::vec2(0.0f, 1.0f),

    glm::vec2(0.0f, 0.0f), glm::vec2(1.0f, 0.0f), glm::vec2(1.0f, 1.0f),
    glm::vec2(1.0f, 1.0f), glm::vec2(0.0f, 1.0f), glm::vec2(0.0f, 0.0f),

    glm::vec2(1.0f, 0.0f), glm::vec2(1.0f, 1.0f), glm::vec2(0.0f, 1.0f),
    glm::vec2(0.0f, 1.0f), glm::vec2(0.0f, 0.0f), glm::vec2(1.0f, 0.0f),

    glm::vec2(1.0f, 0.0f), glm::vec2(0.0f, 1.0f), glm::vec2(1.0f, 1.0f),
    glm::vec2(0.0f, 1.0f), glm::vec2(1.0f, 0.0f), glm::vec2(0.0f, 0.0f),

    glm::vec2(0.0f, 1.0f), glm::vec2(1.0f, 1.0f), glm::vec2(1.0f, 0.0f),
    glm::vec2(1.0f, 0.0f), glm::vec2(0.0f, 0.0f), glm::vec2(0.0f, 1.0f),

    glm::vec2(0.0f, 1.0f), glm::vec2(1.0f, 0.0f), glm::vec2(1.0f, 1.0f),
    glm::vec2(1.0f, 0.0f), glm::vec2(0.0f, 1.0f), glm::vec2(0.0f, 0.0f),
  };

  topology = TOPOLOGY::TRIANGLES;
  Finalize();
}

} // namespace primitives

} // namespace fightingengine