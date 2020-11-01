#pragma once

//c++ standard lib header
#include <map>
#include <memory>

//other library headers
#include <GL/glew.h>

//your project headers
#include "engine/graphics/colour.hpp"
#include "engine/graphics/shading_types.hpp"
#include "engine/graphics/shader.hpp"
#include "engine/graphics/texture.hpp"
#include "engine/graphics/texture_cube.hpp"

namespace fightingengine
{
enum class MaterialType
{
    MATERIAL_DEFAULT,
    MATERIAL_CUSTOM,
    MATERIAL_POST_PROCESS,
};

/* 
    Material object, representing all render state required for rendering a mesh. This includes 
    shader parameters, texture samplers and GL state relevant for rendering a mesh. A material 
    object is required for rendering any scene node. The renderer holds a list of common material
    defaults/templates for deriving or creating new materials.
*/
class Material
{
private:
    // shader state
    std::shared_ptr<Shader> shader_;
    std::map<std::string, UniformValue>        uniforms_;
    std::map<std::string, UniformValueSampler> sampler_uniforms_; 

public:
    MaterialType Type = MaterialType::MATERIAL_CUSTOM;
    ColourVec4f Color;

    // depth state
    bool   DepthTest    = true;
    bool   DepthWrite   = true;
    GLenum DepthCompare = GL_LESS;

    // face culling state
    bool   Cull             = true;
    GLenum CullFace         = GL_BACK;
    GLenum CullWindingOrder = GL_CCW;

    // blending state
    bool   Blend         = false;
    GLenum BlendSrc      = GL_ONE; // pre-multiplied alpha
    GLenum BlendDst      = GL_ONE_MINUS_SRC_ALPHA;
    GLenum BlendEquation = GL_FUNC_ADD;

    // shadow state
    bool ShadowCast    = false;
    bool ShadowReceive = false;

private:
public:
    Material() = default;
    Material(std::shared_ptr<Shader> shader);

    [[nodiscard]] std::shared_ptr<Shader> get_shader();

    void set_bool(std::string name, bool value);
    void set_int(std::string name, int value);
    void set_float(std::string name, float value);
    void set_texture(std::string name, Texture2D* value, unsigned int unit = 0);
    void set_texture_cube(std::string name, TextureCube* value, unsigned int unit = 0);
    void set_vector(std::string name, glm::vec2 value);
    void set_vector(std::string name, glm::vec3 value);
    void set_vector(std::string name, glm::vec4 value);
    void set_matrix(std::string name, glm::mat2 value);
    void set_matrix(std::string name, glm::mat3 value);
    void set_matrix(std::string name, glm::mat4 value);
    
    [[nodiscard]] std::map<std::string, UniformValue>*        get_uniforms();
    [[nodiscard]] std::map<std::string, UniformValueSampler>* get_sampler_uniforms();
};

} //namespace fightingengine

