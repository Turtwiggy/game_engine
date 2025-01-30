// header
#include "engine/opengl/shader.hpp"

// your project headers
#include "engine/app/game_window.hpp"
#include "engine/entt/helpers.hpp"
#include "engine/io/path.hpp"

// this probably shouldnt be here
#include "modules/renderer/components.hpp"

// other library headers
#include "engine/deps/opengl.hpp"
#include <glm/gtc/type_ptr.hpp>

// c++ standard library headers
#include <SDL2/SDL_log.h>
#include <format>
#include <fstream>
#include <sstream>

namespace engine {
using namespace std::literals;

void
check_compile_errors(unsigned int shader, std::string type, std::string path)
{
  GLint success;
  GLchar infoLog[1024];
  if (type != "PROGRAM") {
    glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
    if (!success) {
      glGetShaderInfoLog(shader, 1024, NULL, infoLog);
      SDL_Log("%s",
              std::format("ERROR::SHADER_COMPILATION_ERROR type: {}, path: {} \nlog: {} ", type, path, infoLog).c_str());
    }
  } else {
    glGetProgramiv(shader, GL_LINK_STATUS, &success);
    if (!success) {
      glGetProgramInfoLog(shader, 1024, NULL, infoLog);
      SDL_Log("%s",
              std::format("ERROR::SHADER_COMPILATION_ERROR type: {}, path: {} \nlog: {} ", type, path, infoLog).c_str());
    }
  }
}

void
reload_shader_program(entt::registry& r, unsigned int* id, const std::string& vert_path, const std::string& frag_path)
{
  // Create a new shader program from the given file names. Halt on failure.
  auto new_id = create_opengl_shader(r, vert_path, frag_path);
  // SDL_Log("%s", std::format("reloading shader, new_id: {}", new_id).c_str());

  if (new_id) {
    // SDL_Log("%s", std::format("deleting old shader program").c_str());
    glDeleteProgram(*id);
    *id = new_id;
  }
}

unsigned int
create_opengl_shader(entt::registry& r, const std::string& vert_path, const std::string& frag_path)
{
  // OpenGL ShaderTypes
  // GL_VERTEX_SHADER VERTEX
  // GL_COMPUTE_SHADER COMPUTE
  // GL_FRAGMENT_SHADER FRAGMENT
  // GL_GEOMETRY_SHADER VERTEX

  unsigned int vert_shader = load_shader_from_disk(r, vert_path, GL_VERTEX_SHADER, "VERTEX");
  unsigned int frag_shader = load_shader_from_disk(r, frag_path, GL_FRAGMENT_SHADER, "FRAGMENT");

  unsigned int ID = glCreateProgram();
  glAttachShader(ID, vert_shader);
  glAttachShader(ID, frag_shader);

  glLinkProgram(ID);
  check_compile_errors(ID, "PROGRAM", "");

  glDeleteShader(vert_shader);
  glDeleteShader(frag_shader);

  return ID;
}

unsigned int
load_shader_from_disk(entt::registry& r, const std::string& path, unsigned int gl_shader_type, std::string type)
{
  unsigned int shader_id;
  std::string code;
  {
    const char* compute_shader_path = path.c_str();

    std::ifstream codeFile;
    codeFile.exceptions(std::ifstream::failbit | std::ifstream::badbit);

    try {
      // open files
      codeFile.open(compute_shader_path);

      std::stringstream csShaderStream;

      // read file's buffer contents into streams
      csShaderStream << codeFile.rdbuf();

      // convert stream into string
      code = csShaderStream.str();

      codeFile.close();

    } catch (const std::ifstream::failure& e) {
      SDL_Log("%s", std::format("ERROR::SHADER::FILE_NOT_SUCCESSFULLY_READ {}, info: ", path, e.what()).c_str());

      exit(1);
    }
  }

  // Prepend e.g. "#version 330 core" to the shader.
  std::string version = GameWindow::get_glsl_version() + "\n"s;

  // If the version is e.g. "#version 300 es",
  // we need to specify the float precision
  if (version.find(" es") != std::string::npos)
    version += "precision highp float;\n"s;

  // add the rest of the shader code
  code = version + code;

  const auto& ri_c = game2d::get_first_component<game2d::SINGLE_RendererInfo>(r);

  // Generate keys user for textures
  const auto clean_path = [](const std::string& path) -> std::string {
    const auto last_slash = path.find_last_of("/\\");
    const auto file_name = path.substr(last_slash + 1);
    const auto last_dot = file_name.find_last_of('.');
    return file_name.substr(0, last_dot);
  };
  std::vector<std::string> tex_keys;
  for (const auto& tex : ri_c.user_textures)
    tex_keys.push_back("tex_" + clean_path(tex.path));

  // generate user uniform sampler
  {
    const std::string key0 = "{{ generate_user_samplers }}";
    const size_t pos0 = code.find(key0);
    if (pos0 != std::string::npos) {
      std::string generated = "";

      // generate uniform sampler2D user texture key
      for (const auto& key : tex_keys) {
        generated += "uniform sampler2D ";
        generated += key + "; \n";
      }

      // SDL_Log("generated: %s", generated.c_str());
      code.replace(pos0, key0.length(), generated);
    }
  }

  // generate big if statement for sampling
  {
    const std::string key1 = "{{ generate_sampler_if_statements }}";
    const size_t pos1 = code.find(key1);
    if (pos1 != std::string::npos) {

      const auto get_renderer_tex_unit_count = [&ri_c]() {
        int i = 0;
        for (const auto& p : ri_c.passes)
          i += int(p.texs.size());
        return i;
      };
      const int texs_used_by_renderer = get_renderer_tex_unit_count();

      std::string generated = "";

      for (size_t i = 0; i < tex_keys.size(); i++) {
        auto key = tex_keys[i];
        if (i == 0) {
          // std::string l1 = "out_colour *= tex2dss()";
          std::string l0 = "if(index == RENDERER_TEX_UNIT_COUNT){\n";
          std::string l1 = "out_colour *= tex2dss(" + key + ", sprite_uv, bias, aa_scale);\n";
          // std::string l2 = "return;\n";
          std::string l3 = "}\n";
          generated.append(l0);
          generated.append(l1);
          // generated.append(l2);
          generated.append(l3);
          continue;
        }
        auto i_str = std::to_string(i);
        std::string l0 = "else if(index == RENDERER_TEX_UNIT_COUNT+" + i_str + "){\n";
        std::string l1 = "out_colour *= texture(" + key + ", sprite_uv);\n";
        std::string l2 = "return;\n";
        std::string l3 = "}\n";
        generated.append(l0);
        generated.append(l1);
        generated.append(l2);
        generated.append(l3);
      }

      // SDL_Log("generated: %s", generated.c_str());
      code.replace(pos1, key1.length(), generated);

      // SDL_Log("%s", code.c_str());
    }
  }

  const char* csCode = code.c_str();
  shader_id = glCreateShader(gl_shader_type);
  glShaderSource(shader_id, 1, &csCode, NULL);
  glCompileShader(shader_id);
  check_compile_errors(shader_id, type, path);

  return shader_id;
}

//
// Shader
//

Shader::Shader(entt::registry& r, const std::string& vp, const std::string& fp)
{
  vert_path = get_exe_path_without_exe_name() + vp;
  frag_path = get_exe_path_without_exe_name() + fp;

  ID = create_opengl_shader(r, vert_path, frag_path);
}

void
Shader::bind() const
{
  glUseProgram(ID);
}

void
Shader::unbind() const
{
  glUseProgram(0);
}

void
Shader::reload(entt::registry& r)
{
  reload_shader_program(r, &ID, vert_path, frag_path);
  // SDL_Log("%s", std::format("shader new id: {}", ID).c_str());
}

void
Shader::set_bool(const std::string& name, bool value) const
{
  glUniform1i(glGetUniformLocation(ID, name.c_str()), (int)value);
}
void
Shader::set_int(const std::string& name, int value) const
{
  glUniform1i(glGetUniformLocation(ID, name.c_str()), value);
}
void
Shader::set_int_array(const std::string& name, std::span<const int> values) const
{
  GLint location = glGetUniformLocation(ID, name.c_str());
  int count = (int)values.size();
  glUniform1iv(location, count, values.data());
}
void
Shader::set_uint(const std::string& name, unsigned int value) const
{
  glUniform1ui(glGetUniformLocation(ID, name.c_str()), value);
}
void
Shader::set_float(const std::string& name, float value) const
{
  const auto loc = glGetUniformLocation(ID, name.c_str());
  glUniform1f(loc, value);
}
void
Shader::set_vec2(const std::string& name, const glm::vec2& value) const
{
  glUniform2fv(glGetUniformLocation(ID, name.c_str()), 1, glm::value_ptr(value));
}
void
Shader::set_vec2(const std::string& name, float x, float y) const
{
  glUniform2f(glGetUniformLocation(ID, name.c_str()), x, y);
}
void
Shader::set_vec3(const std::string& name, const glm::vec3& value) const
{
  glUniform3fv(glGetUniformLocation(ID, name.c_str()), 1, glm::value_ptr(value));
}
void
Shader::set_vec3(const std::string& name, float x, float y, float z) const
{
  glUniform3f(glGetUniformLocation(ID, name.c_str()), x, y, z);
}
void
Shader::set_vec4(const std::string& name, const glm::vec4& value) const
{
  glUniform4fv(glGetUniformLocation(ID, name.c_str()), 1, glm::value_ptr(value));
}
void
Shader::set_vec4(const std::string& name, float x, float y, float z, float w)
{
  glUniform4f(glGetUniformLocation(ID, name.c_str()), x, y, z, w);
}
void
Shader::set_mat2(const std::string& name, const glm::mat2& mat) const
{
  glUniformMatrix2fv(glGetUniformLocation(ID, name.c_str()), 1, GL_FALSE, glm::value_ptr(mat));
}
void
Shader::set_mat3(const std::string& name, const glm::mat3& mat) const
{
  glUniformMatrix3fv(glGetUniformLocation(ID, name.c_str()), 1, GL_FALSE, glm::value_ptr(mat));
}
void
Shader::set_mat4(const std::string& name, const glm::mat4& mat) const
{
  glUniformMatrix4fv(glGetUniformLocation(ID, name.c_str()), 1, GL_FALSE, glm::value_ptr(mat));
}

int
Shader::get_uniform_binding_location(const std::string& name) const
{
  int loc = glGetUniformLocation(ID, name.c_str());

  if (loc == -1) {
    SDL_Log("%s", std::format("ERROR: Location of uniform not found: {}", name).c_str());

    return -1;
  }

  int params[1];
  glGetUniformiv(ID, loc, params);
  return params[0];
}

// int
// Shader::get_compute_buffer_bind_location(const std::string& name) const
// {
//   int index = glGetProgramResourceIndex(ID, GL_SHADER_STORAGE_BLOCK, name.c_str());
//   int params[1];
//   GLenum props[1] = { GL_BUFFER_BINDING };
//   glGetProgramResourceiv(ID, GL_SHADER_STORAGE_BLOCK, index, 1, props, 1, NULL, params);
//   return params[0];
// }

// void
// Shader::set_compute_buffer_bind_location(const std::string& name)
// {
//   int index = glGetProgramResourceIndex(ID, GL_SHADER_STORAGE_BLOCK, name.c_str());
//   int location = get_compute_buffer_bind_location(name);
//   glShaderStorageBlockBinding(ID, index, location);
// }

void
Shader::set_uniform_block_binding(const std::string& uniform_name, int binding)
{
  unsigned int idx = glGetUniformBlockIndex(ID, uniform_name.c_str());
  glUniformBlockBinding(ID, idx, binding);
}

} // namespace engine
