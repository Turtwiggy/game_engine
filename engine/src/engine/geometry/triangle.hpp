#pragma once

//c++ standard library headers
#include <vector>

//your project headers
#include "engine/geometry/vertex.hpp"

namespace fightingengine {

struct Triangle
{
public:
    Vertex p0;
    Vertex p1;
    Vertex p2;
};

} //namespace fightingengine
