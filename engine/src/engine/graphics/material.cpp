
//header
#include "engine/graphics/material.hpp"

namespace fightingengine {

Material::Material(std::shared_ptr<Shader> shader)
{
    shader_ = shader;
}

std::shared_ptr<Shader> Material::get_shader()
{
    return shader_;
}

void Material::set_bool(std::string name, bool value)
{
    uniforms_[name].Type = SHADER_TYPE::SHADER_TYPE_BOOL;
    uniforms_[name].Bool = value;
}

void Material::set_int(std::string name, int value)
{
    uniforms_[name].Type = SHADER_TYPE::SHADER_TYPE_INT;
    uniforms_[name].Int  = value;
}

void Material::set_float(std::string name, float value)
{
    uniforms_[name].Type  = SHADER_TYPE::SHADER_TYPE_FLOAT;
    uniforms_[name].Float = value;
}

void Material::set_texture(std::string name, Texture2D* value, unsigned int unit)
{
    sampler_uniforms_[name].Unit    = unit;
    sampler_uniforms_[name].Texture = value;

    switch (value->Target)
    {
    case GL_TEXTURE_1D:
        sampler_uniforms_[name].Type      = SHADER_TYPE::SHADER_TYPE_SAMPLER1D;
        break;
    case GL_TEXTURE_2D:
        sampler_uniforms_[name].Type      = SHADER_TYPE::SHADER_TYPE_SAMPLER2D;
        break;
    case GL_TEXTURE_3D:
        sampler_uniforms_[name].Type      = SHADER_TYPE::SHADER_TYPE_SAMPLER3D;
        break;
    case GL_TEXTURE_CUBE_MAP:
        sampler_uniforms_[name].Type        = SHADER_TYPE::SHADER_TYPE_SAMPLERCUBE;
        break;
    }
    
    if (shader_)
    {
        shader_->bind();
        shader_->set_int(name, unit);
    }
}

void Material::set_texture_cube(std::string name, TextureCube* value, unsigned int unit)
{
    sampler_uniforms_[name].Unit        = unit;
    sampler_uniforms_[name].Type        = SHADER_TYPE::SHADER_TYPE_SAMPLERCUBE;
    sampler_uniforms_[name].TextureCube = value;

    if (shader_)
    {
        shader_->bind();
        shader_->set_int(name, unit);
    }
}

void Material::set_vector(std::string name, glm::vec2 value)
{
    uniforms_[name].Type = SHADER_TYPE::SHADER_TYPE_VEC2;
    uniforms_[name].Vec2 = value;
}

void Material::set_vector(std::string name, glm::vec3 value)
{
    uniforms_[name].Type = SHADER_TYPE::SHADER_TYPE_VEC3;
    uniforms_[name].Vec3 = value;
}

void Material::set_vector(std::string name, glm::vec4 value)
{
    uniforms_[name].Type = SHADER_TYPE::SHADER_TYPE_VEC4;
    uniforms_[name].Vec4 = value;
}

void Material::set_matrix(std::string name, glm::mat2 value)
{
    uniforms_[name].Type = SHADER_TYPE::SHADER_TYPE_MAT2;
    uniforms_[name].Mat2 = value;
}

void Material::set_matrix(std::string name, glm::mat3 value)
{
    uniforms_[name].Type = SHADER_TYPE::SHADER_TYPE_MAT3;
    uniforms_[name].Mat3 = value;
}

void Material::set_matrix(std::string name, glm::mat4 value)
{
    uniforms_[name].Type = SHADER_TYPE::SHADER_TYPE_MAT4;
    uniforms_[name].Mat4 = value;
}

std::map<std::string, UniformValue>*        Material::get_uniforms()
{
    return &uniforms_;
}

std::map<std::string, UniformValueSampler>* Material::get_sampler_uniforms()
{
    return &sampler_uniforms_;
}

} //namespace fightingengine