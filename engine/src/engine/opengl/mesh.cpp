
// header
#include "engine/opengl/mesh.hpp"

// other library headers
#include <gl/glew.h>
#include <glm/glm.hpp>

// c++ libs
#include <iostream>

namespace fightingengine {

// clang-format off
static float vertexPlane[40] =
{
	-1.0f, 1.0f, 0.0f, 1.0f,	0.0f, 0.0f, 1.0f, 0.0f,		0.0f, 1.0f,
	 1.0f,-1.0f, 0.0f, 1.0f,	0.0f, 0.0f, 1.0f, 0.0f,		1.0f, 0.0f,
	 1.0f, 1.0f, 0.0f, 1.0f,	0.0f, 0.0f, 1.0f, 0.0f,		1.0f, 1.0f,
	-1.0f,-1.0f, 0.0f, 1.0f,	0.0f, 0.0f, 1.0f, 0.0f,		0.0f, 0.0f
};

static float vertexCube[] =
{
	-1.0f, 1.0f, 1.0f, 1.0f,	0.0f, 0.0f, 1.0f, 0.0f,			0.0f, 1.0f, // top
	 1.0f,-1.0f, 1.0f, 1.0f,	0.0f, 0.0f, 1.0f, 0.0f,			1.0f, 0.0f,
	 1.0f, 1.0f, 1.0f, 1.0f,	0.0f, 0.0f, 1.0f, 0.0f,			1.0f, 1.0f,
	-1.0f,-1.0f, 1.0f, 1.0f,	0.0f, 0.0f, 1.0f, 0.0f,			0.0f, 0.0f,

	-1.0f, 1.0f, -1.0f, 1.0f,	0.0f, 0.0f, -1.0f, 0.0f,		0.0f, 1.0f, // bottom
	 1.0f,-1.0f, -1.0f, 1.0f,	0.0f, 0.0f, -1.0f, 0.0f,		1.0f, 0.0f,
	 1.0f, 1.0f, -1.0f, 1.0f,	0.0f, 0.0f, -1.0f, 0.0f,		1.0f, 1.0f,
	-1.0f,-1.0f, -1.0f, 1.0f,	0.0f, 0.0f, -1.0f, 0.0f,		0.0f, 0.0f,

	 -1.0f, -1.0f, 1.0f,1.0f,	-1.0f, 0.0f, 0.0f, 0.0f,		0.0f, 1.0f, // left
	 -1.0f,  1.0f,-1.0f,1.0f,	-1.0f, 0.0f, 0.0f, 0.0f,		1.0f, 0.0f,
	 -1.0f,  1.0f, 1.0f,1.0f,	-1.0f, 0.0f, 0.0f, 0.0f,		1.0f, 1.0f,
	 -1.0f, -1.0f,-1.0f,1.0f,	-1.0f, 0.0f, 0.0f, 0.0f,		0.0f, 0.0f,

	 1.0f, -1.0f, 1.0f,1.0f,	1.0f, 0.0f, 0.0f, 0.0f,			0.0f, 1.0f, // right
	 1.0f,  1.0f,-1.0f,1.0f,	1.0f, 0.0f, 0.0f, 0.0f,			1.0f, 0.0f,
	 1.0f,  1.0f, 1.0f,1.0f,	1.0f, 0.0f, 0.0f, 0.0f,			1.0f, 1.0f,
	 1.0f, -1.0f,-1.0f,1.0f,	1.0f, 0.0f, 0.0f, 0.0f,			0.0f, 0.0f,

	-1.0f, 1.0f,  1.0f,1.0f,	0.0f, 1.0f, 0.0f, 0.0f,			0.0f, 1.0f, // front
	 1.0f, 1.0f, -1.0f,1.0f,	0.0f, 1.0f, 0.0f, 0.0f,			1.0f, 0.0f,
	 1.0f, 1.0f,  1.0f,1.0f,	0.0f, 1.0f, 0.0f, 0.0f,			1.0f, 1.0f,
	-1.0f, 1.0f, -1.0f,1.0f,	0.0f, 1.0f, 0.0f, 0.0f,			0.0f, 0.0f,

	-1.0f, -1.0f,  1.0f,1.0f,	0.0f, -1.0f, 0.0f, 0.0f,		0.0f, 1.0f, // back
	 1.0f, -1.0f, -1.0f,1.0f,	0.0f, -1.0f, 0.0f, 0.0f,		1.0f, 0.0f,
	 1.0f, -1.0f,  1.0f,1.0f,	0.0f, -1.0f, 0.0f, 0.0f,		1.0f, 1.0f,
	-1.0f, -1.0f, -1.0f,1.0f,	0.0f, -1.0f, 0.0f, 0.0f,		0.0f, 0.0f,
};

static unsigned short indexPlane[6]
{
	0+0, 0+2, 0+1,
	0+0, 0+1, 0+3
};
// clang-format on

Mesh::Mesh(std::vector<Vertex> vertices, std::vector<unsigned int> indices)
{
  this->verts = vertices;
  this->indices = indices;

  setup_mesh();
}

void
Mesh::setup_mesh()
{
  // initialize object IDs
  glGenVertexArrays(1, &vao);
  glGenBuffers(1, &vbo);
  glGenBuffers(1, &ebo);

  glBindVertexArray(vao);
  // load data into vertex buffers
  glBindBuffer(GL_ARRAY_BUFFER, vbo);
  glBufferData(GL_ARRAY_BUFFER, verts.size() * sizeof(Vertex), &verts[0], GL_STATIC_DRAW);

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

  // unbind
  glBindVertexArray(0);
}

void
Mesh::draw(Shader& shader)
{
  // bind vao
  glBindVertexArray(vao);
  glDrawElements(GL_TRIANGLES, indices.size(), GL_UNSIGNED_INT, 0);
  glBindVertexArray(0);
  glActiveTexture(GL_TEXTURE0);
}

} // namespace fightingengine
