#pragma once

#include "engine/graphics/texture_cube.hpp"
#include "engine/graphics/shader.hpp"
#include "engine/mesh/mesh.hpp"

namespace fightingengine {

class Background : public SceneNode
{
private:
    TextureCube* texture_cube_;
    Shader* shader_;

public:
    Background();
    ~Background() = default;

    void set_cubemap(TextureCube* cubemap);
};

} //namespace fightingengine