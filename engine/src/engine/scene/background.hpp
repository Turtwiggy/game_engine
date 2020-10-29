#pragma once

#include <memory>

#include "engine/scene/scene_node.hpp"
#include "engine/graphics/texture_cube.hpp"
#include "engine/graphics/shader.hpp"
#include "engine/mesh/mesh.hpp"

namespace fightingengine {

class Background : public SceneNode
{
private:
    std::unique_ptr<TextureCube> texture_cube_;
    std::shared_ptr<Shader> shader_;

public:
    Background();
    ~Background() = default;

    //void set_cubemap(TextureCube* cubemap);
};

} //namespace fightingengine