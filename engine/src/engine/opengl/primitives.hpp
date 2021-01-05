#pragma once

#include "engine/opengl/mesh.hpp"

namespace fightingengine {

void
render_mesh(std::shared_ptr<Mesh> mesh);
void
render_mesh(Mesh& mesh);

namespace primitives {

class Plane : public Mesh
{
public:
  Plane() = default;
  Plane(int x, int y);
};

class Cube : public Mesh
{
public:
  Cube();
};

} // namespace primitives

} // namespace fightingengine