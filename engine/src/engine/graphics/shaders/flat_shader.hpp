#pragma once

//c++ standard library headers
#include <string>

//your project headers
#include "engine/graphics/shader.hpp"

namespace fightingengine {

//Shader parameters

// A directional light
// glm::vec3 lightColor(0.9f, 0.9f, 0.9f);
// glm::vec3 diffuseColor = lightColor   * glm::vec3(0.5f); // decrease the influence
// glm::vec3 ambientColor = diffuseColor * glm::vec3(0.2f); // low influence
// flat_shader_.get_shader().setVec3("light.ambient", ambientColor);
// flat_shader_.get_shader().setVec3("light.diffuse", diffuseColor);
// flat_shader_.get_shader().setVec3("light.specular", 1.0f, 1.0f, 1.0f);
// flat_shader_.get_shader().setVec3("light.direction", -0.2f, -1.0f, -0.3f);

// model
// glm::mat4 model = glm::mat4(1.0f);
// model = glm::translate(model, glm::vec3(0.0f, 0.0f, 0.0f));
// model = glm::scale(model, glm::vec3(1.0f, 1.0f, 1.0f));
// flat_shader_.get_shader().setMat4("model", model);

// material
// shader.setVec3("material.ambient", this->colour.colour);
// shader.setVec3("material.diffuse", this->colour.colour);
// shader.setVec3("material.specular", 0.5f, 0.5f, 0.5f);
// shader.setFloat("material.shininess", 32.0f);

class FlatShader
{
public:
    FlatShader() = default;

    void load();

    [[nodiscard]] Shader get_shader() const;

private: 
    Shader shader_;
};

} //namespace fightingengine