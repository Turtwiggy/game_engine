#pragma once

//c++ standard lib header
#include <string>

//your project headers
#include "engine/graphics/texture.hpp"
#include "engine/graphics/texture_cube.hpp"

namespace fightingengine {

enum SHADER_TYPE
{
    SHADER_TYPE_BOOL,
    SHADER_TYPE_INT,
    SHADER_TYPE_FLOAT,
    SHADER_TYPE_SAMPLER1D,
    SHADER_TYPE_SAMPLER2D,
    SHADER_TYPE_SAMPLER3D,
    SHADER_TYPE_SAMPLERCUBE,
    SHADER_TYPE_VEC2,
    SHADER_TYPE_VEC3,
    SHADER_TYPE_VEC4,
    SHADER_TYPE_MAT2,
    SHADER_TYPE_MAT3,
    SHADER_TYPE_MAT4,
};

struct Uniform
{
    SHADER_TYPE  Type;
    std::string  Name;
    int          Size;
    unsigned int Location;
};

struct UniformValue
{
    SHADER_TYPE Type;
    // each element takes up the space of its largest 
    // element (mat4) which is 64 bytes
    union
    {
        bool       Bool;
        int        Int;
        float      Float;

        glm::vec2 Vec2;
        glm::vec3 Vec3;
        glm::vec4 Vec4;
        glm::mat2 Mat2;
        glm::mat3 Mat3;
        glm::mat4 Mat4;
    };

    UniformValue() {}
};

struct UniformValueSampler
{
    SHADER_TYPE  Type;
    unsigned int Unit;
    union 
    {
        Texture2D   *Texture;
        TextureCube *TextureCube;
    };

    UniformValueSampler() {}
};

struct VertexAttribute
{
    SHADER_TYPE  Type;
    std::string  Name;
    int          Size;
    unsigned int Location;
};

} //namespace fightingengine