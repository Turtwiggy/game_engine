#pragma once

#include "engine/mesh/mesh.hpp"

namespace fightingengine {

namespace primitives {

class Plane : public Mesh {
public:
    Plane(unsigned int xSegments, unsigned int ySegments);
};

class Cube : public Mesh {
public:
    Cube();
};

} //namespace primitives

} //namespace fightingengine