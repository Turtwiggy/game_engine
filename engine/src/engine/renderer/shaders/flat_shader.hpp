#pragma once

//c++ standard library headers
#include <string>

//your project headers
#include "engine/renderer/shader.hpp"

namespace fightingengine {

class FlatShader
{
public:
    FlatShader() = default;

    void load(
        const std::string& path = "assets/shaders/blinn-phong/", 
        const std::string& vert_name = "lit.vert", 
        const std::string& frag_name = "lit.frag" );

    [[nodiscard]] Shader get_shader() const;

private: 
    Shader shader_;

};

} //namespace fightingengine