
// header
#include "engine/opengl/mesh.hpp"

// c++ libs
#include <iostream>

// other library headers
#include <gl/glew.h>
#include <glm/glm.hpp>

namespace fightingengine {

void
Mesh::setup_mesh(bool interleaved)
{
  // initialize object IDs if not configured before
  if (!vao) {
    glGenVertexArrays(1, &vao);
    glGenBuffers(1, &vbo);
    glGenBuffers(1, &ebo);
  }

  glBindVertexArray(vao);
  glBindBuffer(GL_ARRAY_BUFFER, vbo);

  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
  glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), &indices[0], GL_STATIC_DRAW);

  // vertex positions
  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)0);
  glEnableVertexAttribArray(0);
  // vertex normals
  glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, normal));
  glEnableVertexAttribArray(1);
  // vertex texture coords
  glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, tex_coords));
  glEnableVertexAttribArray(2);

  glBindVertexArray(0);
}

void
render_mesh(std::shared_ptr<Mesh> mesh)
{
  // bind vao
  glBindVertexArray(mesh->vao);

  GLenum t = mesh->topology == TOPOLOGY::TRIANGLE_STRIP ? GL_TRIANGLE_STRIP : GL_TRIANGLES;

  if (mesh->indices.size() > 0) {
    glDrawElements(t, mesh->indices.size(), GL_UNSIGNED_INT, 0);
  } else {
    glDrawArrays(t, 0, mesh->verts.size());
  }

  glBindVertexArray(0);
}

void
render_mesh(Mesh& mesh)
{
  // bind vao
  glBindVertexArray(mesh.vao);

  GLenum t = mesh.topology == TOPOLOGY::TRIANGLE_STRIP ? GL_TRIANGLE_STRIP : GL_TRIANGLES;

  if (mesh.indices.size() > 0) {
    glDrawElements(t, mesh.indices.size(), GL_UNSIGNED_INT, 0);
  } else {
    glDrawArrays(t, 0, mesh.verts.size());
  }

  glBindVertexArray(0);
}

namespace primitives {

// ---- plane

// Plane::Plane()
// {
//   mesh.verts = std::vector<Vertex>();

//   mesh.verts.position =
//     std::vector<glm::vec3>{ glm::vec3(1.0f, 0.0f, 1.0f), glm::vec3(-1.0f, 0.0f, 1.0f),  glm::vec3(-1.0f, 0.0f,
//     -1.0f),
//                             glm::vec3(1.0f, 0.0f, 1.0f), glm::vec3(-1.0f, 0.0f, -1.0f), glm::vec3(1.0f, 0.0f, -1.0f)
//                             };

//   mesh.verts.normal =
//     std::vector<glm::vec3>{ glm::vec3(0.0f, 1.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f),

//                             glm::vec3(0.0f, 1.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f) };

//   mesh.verts.tex_coords = std::vector<glm::vec2>{
//     glm::vec2(1.0f, 0.0f), glm::vec2(0.0f, 0.0f), glm::vec2(0.0f, 1.0f),

//     glm::vec2(1.0f, 0.0f), glm::vec2(0.0f, 1.0f), glm::vec2(1.0f, 1.0f),
//   };

//   mesh.topology = TOPOLOGY::TRIANGLE_STRIP;
//   mesh.Finalize();
// }

// ---- cube

// Cube::Cube()
// {
//   mesh.verts = std::vector<Vertex>();

//   mesh.verts.position = std::vector<glm::vec3>{
//     glm::vec3(-1.0f, -1.0f, -1.0f), glm::vec3(1.0f, 1.0f, -1.0f),   glm::vec3(1.0f, -1.0f, -1.0f),
//     glm::vec3(1.0f, 1.0f, -1.0f),   glm::vec3(-1.0f, -1.0f, -1.0f), glm::vec3(-1.0f, 1.0f, -1.0f),

//     glm::vec3(-1.0f, -1.0f, 1.0f),  glm::vec3(1.0f, -1.0f, 1.0f),   glm::vec3(1.0f, 1.0f, 1.0f),
//     glm::vec3(1.0f, 1.0f, 1.0f),    glm::vec3(-1.0f, 1.0f, 1.0f),   glm::vec3(-1.0f, -1.0f, 1.0f),

//     glm::vec3(-1.0f, 1.0f, 1.0f),   glm::vec3(-1.0f, 1.0f, -1.0f),  glm::vec3(-1.0f, -1.0f, -1.0f),
//     glm::vec3(-1.0f, -1.0f, -1.0f), glm::vec3(-1.0f, -1.0f, 1.0f),  glm::vec3(-1.0f, 1.0f, 1.0f),

//     glm::vec3(1.0f, 1.0f, 1.0f),    glm::vec3(1.0f, -1.0f, -1.0f),  glm::vec3(1.0f, 1.0f, -1.0f),
//     glm::vec3(1.0f, -1.0f, -1.0f),  glm::vec3(1.0f, 1.0f, 1.0f),    glm::vec3(1.0f, -1.0f, 1.0f),

//     glm::vec3(-1.0f, -1.0f, -1.0f), glm::vec3(1.0f, -1.0f, -1.0f),  glm::vec3(1.0f, -1.0f, 1.0f),
//     glm::vec3(1.0f, -1.0f, 1.0f),   glm::vec3(-1.0f, -1.0f, 1.0f),  glm::vec3(-1.0f, -1.0f, -1.0f),

//     glm::vec3(-1.0f, 1.0f, -1.0f),  glm::vec3(1.0f, 1.0f, 1.0f),    glm::vec3(1.0f, 1.0f, -1.0f),
//     glm::vec3(1.0f, 1.0f, 1.0f),    glm::vec3(-1.0f, 1.0f, -1.0f),  glm::vec3(-1.0f, 1.0f, 1.0f)
//   };

//   mesh.verts.normal = std::vector<glm::vec3>{

//     glm::vec3(0.0f, 0.0f, -1.0f), glm::vec3(0.0f, 0.0f, -1.0f), glm::vec3(0.0f, 0.0f, -1.0f),
//     glm::vec3(0.0f, 0.0f, -1.0f), glm::vec3(0.0f, 0.0f, -1.0f), glm::vec3(0.0f, 0.0f, -1.0f),

//     glm::vec3(0.0f, 0.0f, 1.0f),  glm::vec3(0.0f, 0.0f, 1.0f),  glm::vec3(0.0f, 0.0f, 1.0f),
//     glm::vec3(0.0f, 0.0f, 1.0f),  glm::vec3(0.0f, 0.0f, 1.0f),  glm::vec3(0.0f, 0.0f, 1.0f),

//     glm::vec3(-1.0f, 0.0f, 0.0f), glm::vec3(-1.0f, 0.0f, 0.0f), glm::vec3(-1.0f, 0.0f, 0.0f),
//     glm::vec3(-1.0f, 0.0f, 0.0f), glm::vec3(-1.0f, 0.0f, 0.0f), glm::vec3(-1.0f, 0.0f, 0.0f),

//     glm::vec3(1.0f, 0.0f, 0.0f),  glm::vec3(1.0f, 0.0f, 0.0f),  glm::vec3(1.0f, 0.0f, 0.0f),
//     glm::vec3(1.0f, 0.0f, 0.0f),  glm::vec3(1.0f, 0.0f, 0.0f),  glm::vec3(1.0f, 0.0f, 0.0f),

//     glm::vec3(0.0f, -1.0f, 0.0f), glm::vec3(0.0f, -1.0f, 0.0f), glm::vec3(0.0f, -1.0f, 0.0f),
//     glm::vec3(0.0f, -1.0f, 0.0f), glm::vec3(0.0f, -1.0f, 0.0f), glm::vec3(0.0f, -1.0f, 0.0f),

//     glm::vec3(0.0f, 1.0f, 0.0f),  glm::vec3(0.0f, 1.0f, 0.0f),  glm::vec3(0.0f, 1.0f, 0.0f),
//     glm::vec3(0.0f, 1.0f, 0.0f),  glm::vec3(0.0f, 1.0f, 0.0f),  glm::vec3(0.0f, 1.0f, 0.0f)
//   };

//   mesh.verts.tex_coords = std::vector<glm::vec2>{
//     glm::vec2(0.0f, 0.0f), glm::vec2(1.0f, 1.0f), glm::vec2(1.0f, 0.0f),
//     glm::vec2(1.0f, 1.0f), glm::vec2(0.0f, 0.0f), glm::vec2(0.0f, 1.0f),

//     glm::vec2(0.0f, 0.0f), glm::vec2(1.0f, 0.0f), glm::vec2(1.0f, 1.0f),
//     glm::vec2(1.0f, 1.0f), glm::vec2(0.0f, 1.0f), glm::vec2(0.0f, 0.0f),

//     glm::vec2(1.0f, 0.0f), glm::vec2(1.0f, 1.0f), glm::vec2(0.0f, 1.0f),
//     glm::vec2(0.0f, 1.0f), glm::vec2(0.0f, 0.0f), glm::vec2(1.0f, 0.0f),

//     glm::vec2(1.0f, 0.0f), glm::vec2(0.0f, 1.0f), glm::vec2(1.0f, 1.0f),
//     glm::vec2(0.0f, 1.0f), glm::vec2(1.0f, 0.0f), glm::vec2(0.0f, 0.0f),

//     glm::vec2(0.0f, 1.0f), glm::vec2(1.0f, 1.0f), glm::vec2(1.0f, 0.0f),
//     glm::vec2(1.0f, 0.0f), glm::vec2(0.0f, 0.0f), glm::vec2(0.0f, 1.0f),

//     glm::vec2(0.0f, 1.0f), glm::vec2(1.0f, 0.0f), glm::vec2(1.0f, 1.0f),
//     glm::vec2(1.0f, 0.0f), glm::vec2(0.0f, 1.0f), glm::vec2(0.0f, 0.0f),
//   };

//   mesh.topology = TOPOLOGY::TRIANGLES;
//   mesh.Finalize();
// }

} // namespace primitives

} // namespace fightingengine
