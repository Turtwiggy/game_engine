// header
#include "engine/opengl/shader.hpp"

// your project headers
#include "engine/opengl/util.hpp"

// other library headers
#include <GL/glew.h>

// c++ standard library headers
#include <filesystem> // C++17
#include <fstream>
#include <sstream>

namespace engine {

void
check_compile_errors(unsigned int shader, std::string type)
{
  GLint success;
  GLchar infoLog[1024];
  if (type != "PROGRAM") {
    glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
    if (!success) {
      glGetShaderInfoLog(shader, 1024, NULL, infoLog);
      std::cout << "ERROR::SHADER_COMPILATION_ERROR of type: " << type << "\n"
                << infoLog << "\n -- --------------------------------------------------- -- " << std::endl;
    }
  } else {
    glGetProgramiv(shader, GL_LINK_STATUS, &success);
    if (!success) {
      glGetProgramInfoLog(shader, 1024, NULL, infoLog);
      std::cout << "ERROR::PROGRAM_LINKING_ERROR of type: " << type << "\n"
                << infoLog << "\n -- --------------------------------------------------- -- " << std::endl;
    }
  }
}

void
reload_shader_program(unsigned int* id, const std::string& vert_path, const std::string& frag_path)
{
  printf("Reloading shader: %s %s\n", vert_path.c_str(), frag_path.c_str());

  // Create a new shader program from the given file names. Halt on failure.
  unsigned int new_id = create_opengl_shader(vert_path, frag_path);

  if (new_id) {
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
  check_compile_errors(ID, "PROGRAM");

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
    } catch (const std::ifstream::failure& e) {
      std::cout << "ERROR::SHADER::FILE_NOT_SUCCESFULLY_READ " << path << std::endl;
      exit(1);
    }
  }

  const char* csCode = code.c_str();
  shader_id = glCreateShader(gl_shader_type);
  glShaderSource(shader_id, 1, &csCode, NULL);
  glCompileShader(shader_id);
  check_compile_errors(shader_id, type);

  return shader_id;
}

//
// Shader
//

Shader::Shader(const std::string& vert_path, const std::string& frag_path)
  : vert_path(vert_path)
  , frag_path(frag_path)
{
  ID = create_opengl_shader(vert_path, frag_path);
}

void
Shader::bind()
{
  glUseProgram(ID);
}

void
Shader::unbind()
{
  glUseProgram(0);
}

void
Shader::reload()
{
  reload_shader_program(&ID, vert_path, frag_path);
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
Shader::set_int_array(const std::string& name, int* values, int size) const
{
  glUniform1iv(glGetUniformLocation(ID, name.c_str()), size, values);
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
  glUniform2fv(glGetUniformLocation(ID, name.c_str()), 1, &value[0]);
}
void
Shader::set_vec2(const std::string& name, float x, float y) const
{
  glUniform2f(glGetUniformLocation(ID, name.c_str()), x, y);
}
void
Shader::set_vec3(const std::string& name, const glm::vec3& value) const
{
  glUniform3fv(glGetUniformLocation(ID, name.c_str()), 1, &value[0]);
}
void
Shader::set_vec3(const std::string& name, float x, float y, float z) const
{
  glUniform3f(glGetUniformLocation(ID, name.c_str()), x, y, z);
}
void
Shader::set_vec4(const std::string& name, const glm::vec4& value) const
{
  glUniform4fv(glGetUniformLocation(ID, name.c_str()), 1, &value[0]);
}
void
Shader::set_vec4(const std::string& name, float x, float y, float z, float w)
{
  glUniform4f(glGetUniformLocation(ID, name.c_str()), x, y, z, w);
}
void
Shader::set_mat2(const std::string& name, const glm::mat2& mat) const
{
  glUniformMatrix2fv(glGetUniformLocation(ID, name.c_str()), 1, GL_FALSE, &mat[0][0]);
}
void
Shader::set_mat3(const std::string& name, const glm::mat3& mat) const
{
  glUniformMatrix3fv(glGetUniformLocation(ID, name.c_str()), 1, GL_FALSE, &mat[0][0]);
}
void
Shader::set_mat4(const std::string& name, const glm::mat4& mat) const
{
  glUniformMatrix4fv(glGetUniformLocation(ID, name.c_str()), 1, GL_FALSE, &mat[0][0]);
}

int
Shader::get_uniform_binding_location(const std::string& name) const
{
  int loc = glGetUniformLocation(ID, name.c_str());

  if (loc == -1) {
    printf("ERROR: Location of uniform not found: %s \n", name.c_str());
    return -1;
  }

  int params[1];
  glGetUniformiv(ID, loc, params);
  return params[0];
}

int
Shader::get_compute_buffer_binding_location(const std::string& name) const
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

  int location = get_compute_buffer_binding_location(name);

  glShaderStorageBlockBinding(ID, index, location);
}

} // namespace engine
