#pragma once

// c++ standard library headers
#include <iostream>
#include <string>
#include <vector>

// other library headers
#include <glm/glm.hpp>

namespace fightingengine {

void
check_compile_errors(unsigned int shader, std::string type);

void
reload_shader_program(unsigned int* id, const std::string& vert_path, const std::string& frag_path);

[[nodiscard]] unsigned int
create_opengl_shader(const std::string& vert_path, const std::string& frag_path);

[[nodiscard]] unsigned int
load_shader_from_disk(const std::string& path, unsigned int gl_shader_type, std::string type);

class Shader
{
public:
  unsigned int ID;

  Shader(const std::string& vert_path, const std::string& frag_path);

  void bind();
  void unbind();

  void set_bool(const std::string& name, bool value) const;
  void set_int(const std::string& name, int value) const;
  void set_int_array(const std::string& name, int values[]) const;
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

  [[nodiscard]] int get_uniform_binding_location(const std::string& name) const;

  [[nodiscard]] int get_compute_buffer_binding_location(const std::string& name) const;
  void set_compute_buffer_bind_location(const std::string& name);

private:
};

} // namespace fightingengine
