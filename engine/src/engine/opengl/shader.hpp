#pragma once

// c++ standard library headers
#include <iostream>
#include <string>
#include <vector>

// other library headers
#include <glm/glm.hpp>

namespace fightingengine {

enum class OpenGLShaderTypes
{
  VERTEX,
  FRAGMENT,
  COMPUTE,
  GEOMETRY
};

class Shader
{
public:
  unsigned int ID;

  Shader() = default;

  Shader& attach_shader(const std::string& path, OpenGLShaderTypes shader_type);
  Shader& build_program();

  // gets the type based off the extention
  // e.g. hello.vert returns OpenGLShaderTypes::VERTEX
  // e.g. hello.frag returns OpenGLShaderTypes::FRAGMENT
  // e.g. hello.glsl returns OpenGLShaderTypes::COMPUTE
  static OpenGLShaderTypes convert_file_to_shadertype(std::string file);

  // activate the shader
  void bind();
  void unbind();

  // utility uniform functions
  [[nodiscard]] int get_uniform_binding_location(const std::string& name) const;
  void set_bool(const std::string& name, bool value) const;
  void set_int(const std::string& name, int value) const;
  void set_uint(const std::string& name, unsigned int value) const;
  void set_float(const std::string& name, float value) const;
  void set_vec2(const std::string& name, const glm::vec2& value) const;
  void set_vec2(const std::string& name, float x, float y) const;
  void set_vec3(const std::string& name, const glm::vec3& value) const;
  void set_vec3(const std::string& name, float x, float y, float z) const;
  void set_vec4(const std::string& name, const glm::vec4& value) const;
  void set_vec4(const std::string& name, float x, float y, float z, float w);
  void set_mat2(const std::string& name, const glm::mat2& mat) const;
  void set_mat3(const std::string& name, const glm::mat3& mat) const;
  void set_mat4(const std::string& name, const glm::mat4& mat) const;

  [[nodiscard]] int get_compute_buffer_binding_location(const std::string& name) const;
  void set_compute_buffer_bind_location(const std::string& name);

private:
  // this is the latest path added via attach_shader()
  // this is mainly used for debugging
  std::string latest_path;

  bool ok_to_build = true;

  std::vector<unsigned int> shaders;

  unsigned int load_shader(const std::string& path, unsigned int gl_shader_type, std::string type);

  void attach_shaders_to_program();

  // utility function for checking shader compilation/linking errors.
  void check_compile_errors(unsigned int shader, std::string type);
};

} // namespace fightingengine
