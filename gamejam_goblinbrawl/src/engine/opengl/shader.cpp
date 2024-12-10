// header
#include "engine/opengl/shader.hpp"

// your project headers
#include "engine/app/game_window.hpp"
#include "engine/io/path.hpp"
#include "engine/opengl/util.hpp"

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
reload_shader_program(unsigned int* id, const std::string& vert_path, const std::string& frag_path)
{
  // Create a new shader program from the given file names. Halt on failure.
  auto new_id = create_opengl_shader(vert_path, frag_path);
  // SDL_Log("%s", std::format("reloading shader, new_id: {}", new_id).c_str());

  if (new_id) {
    // SDL_Log("%s", std::format("deleting old shader program").c_str());
    glDeleteProgram(*id);
    *id = new_id;
  }
}

unsigned int
create_opengl_shader(const std::string& vert_path, const std::string& frag_path)
{
  // OpenGL ShaderTypes
  // GL_VERTEX_SHADER VERTEX
  // GL_COMPUTE_SHADER COMPUTE
  // GL_FRAGMENT_SHADER FRAGMENT
  // GL_GEOMETRY_SHADER VERTEX

  unsigned int vert_shader = load_shader_from_disk(vert_path, GL_VERTEX_SHADER, "VERTEX");
  unsigned int frag_shader = load_shader_from_disk(frag_path, GL_FRAGMENT_SHADER, "FRAGMENT");

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
load_shader_from_disk(const std::string& path, unsigned int gl_shader_type, std::string type)
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

Shader::Shader(const std::string& vp, const std::string& fp)
{
  vert_path = get_exe_path_without_exe_name() + vp;
  frag_path = get_exe_path_without_exe_name() + fp;

  ID = create_opengl_shader(vert_path, frag_path);
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
Shader::reload()
{
  reload_shader_program(&ID, vert_path, frag_path);
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
  glUniform1f(glGetUniformLocation(ID, name.c_str()), value);
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

int
Shader::get_compute_buffer_bind_location(const std::string& name) const
{
  int index = glGetProgramResourceIndex(ID, GL_SHADER_STORAGE_BLOCK, name.c_str());
  int params[1];
  GLenum props[1] = { GL_BUFFER_BINDING };
  glGetProgramResourceiv(ID, GL_SHADER_STORAGE_BLOCK, index, 1, props, 1, NULL, params);
  return params[0];
}

void
Shader::set_compute_buffer_bind_location(const std::string& name)
{
  int index = glGetProgramResourceIndex(ID, GL_SHADER_STORAGE_BLOCK, name.c_str());

  int location = get_compute_buffer_bind_location(name);

  glShaderStorageBlockBinding(ID, index, location);
}

} // namespace engine
