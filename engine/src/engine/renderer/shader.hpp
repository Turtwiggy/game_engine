#pragma once

#include "engine/renderer/util/util_functions.hpp"

#include <glm/glm.hpp>
#include <GL/glew.h>

#include <vector>
#include <string>
#include <fstream>
#include <sstream>
#include <iostream>
#include <filesystem> // C++17
namespace fs = std::filesystem;

namespace fightingengine {

    enum OpenGLShaderTypes
    {
        VERTEX,
        FRAGMENT,
        COMPUTE,
        GEOMETRY
    };

    class Shader
    {
    public:
        Shader() = default;

        unsigned int ID;

        Shader& attach_shader(const char* path, OpenGLShaderTypes shader_type)
        {
            std::string name;
            unsigned int type;

            switch (shader_type)
            {
            case VERTEX:
                name = "VERTEX";
                type = GL_VERTEX_SHADER;
                break;
            case COMPUTE:
                name = "COMPUTE";
                type = GL_COMPUTE_SHADER;
                break;
            case FRAGMENT:
                name = "FRAGMENT";
                type = GL_FRAGMENT_SHADER;
                break;
            case GEOMETRY:
                name = "GEOMETRY";
                type = GL_GEOMETRY_SHADER;
                break;
            default:
                //ok to have a default as it wont compile anyway
                name = "VERTEX";
                type = GL_VERTEX_SHADER;
                break;
            }

            printf("(Shader) attaching shader: %s \n", path);
            unsigned int shader = load_shader(path, type, name);

            shaders.push_back(shader);

            return *this;
        }

        Shader& build_program()
        {
            if (ok_to_build) {
                attach_shaders_to_program();
                printf("OK! to build program \n");
            }
            else {
                printf("SHADER ERROR: program was not ok to build \n");
            }

            return *this;
        }

        static OpenGLShaderTypes convert_file_to_shadertype(std::string file)
        {
            fs::path p(file.c_str());
            std::string extention = p.extension().generic_string();

            if (extention == ".vert")
            {
                return VERTEX;
            }
            else if (extention == ".frag")
            {
                return FRAGMENT;
            }
            else if (extention == ".glsl")
            {
                return COMPUTE;
            }

            printf("ERROR: convert_file_to_shadertype %s extention not found \n", extention.c_str());
            return VERTEX;
        }

    private:

        bool ok_to_build = true;
        std::vector<unsigned int> shaders;

        unsigned int load_shader(const char* path, unsigned int gl_shader_type, std::string type)
        {
            unsigned int shader_id;
            std::string code;
            {
                const char* compute_shader_path = path;

                std::ifstream codeFile;
                codeFile.exceptions(std::ifstream::failbit | std::ifstream::badbit);

                try {
                    // open files
                    codeFile.open(compute_shader_path);

                    std::stringstream csShaderStream;

                    // read file's buffer contents into streams
                    csShaderStream << codeFile.rdbuf();

                    // close file handlers
                    codeFile.close();

                    // convert stream into string
                    code = csShaderStream.str();
                }
                catch (std::ifstream::failure e)
                {
                    std::cout << "ERROR::SHADER::FILE_NOT_SUCCESFULLY_READ" << std::endl;
                    ok_to_build = false;
                }
            }

            const char* csCode = code.c_str();
            shader_id = glCreateShader(gl_shader_type);
            glShaderSource(shader_id, 1, &csCode, NULL);
            glCompileShader(shader_id);
            Shader::check_compile_errors(shader_id, type);

            return shader_id;
        }

        void attach_shaders_to_program()
        {
            ID = glCreateProgram();

            for (unsigned int& a : shaders)
            {
                glAttachShader(ID, a);
            }

            glLinkProgram(ID);
            check_compile_errors(ID, "PROGRAM");

            for (unsigned int& a : shaders)
            {
                glDeleteShader(a);
            }
        }

    public:

        // activate the shader

        void bind()
        {
            glUseProgram(ID);
        }

        void unbind()
        {
            glUseProgram(0);
        }

        // utility uniform functions

        void setBool(const std::string& name, bool value) const
        {
            glUniform1i(glGetUniformLocation(ID, name.c_str()), (int)value);
        }
        void setInt(const std::string& name, int value) const
        {
            glUniform1i(glGetUniformLocation(ID, name.c_str()), value);
        }
        void setFloat(const std::string& name, float value) const
        {
            glUniform1f(glGetUniformLocation(ID, name.c_str()), value);
        }
        void setVec2(const std::string& name, const glm::vec2& value) const
        {
            glUniform2fv(glGetUniformLocation(ID, name.c_str()), 1, &value[0]);
        }
        void setVec2(const std::string& name, float x, float y) const
        {
            glUniform2f(glGetUniformLocation(ID, name.c_str()), x, y);
        }
        void setVec3(const std::string& name, const glm::vec3& value) const
        {
            glUniform3fv(glGetUniformLocation(ID, name.c_str()), 1, &value[0]);
        }
        void setVec3(const std::string& name, float x, float y, float z) const
        {
            glUniform3f(glGetUniformLocation(ID, name.c_str()), x, y, z);
        }
        void setVec4(const std::string& name, const glm::vec4& value) const
        {
            glUniform4fv(glGetUniformLocation(ID, name.c_str()), 1, &value[0]);
        }
        void setVec4(const std::string& name, float x, float y, float z, float w)
        {
            glUniform4f(glGetUniformLocation(ID, name.c_str()), x, y, z, w);
        }
        void setMat2(const std::string& name, const glm::mat2& mat) const
        {
            glUniformMatrix2fv(glGetUniformLocation(ID, name.c_str()), 1, GL_FALSE, &mat[0][0]);
        }
        void setMat3(const std::string& name, const glm::mat3& mat) const
        {
            glUniformMatrix3fv(glGetUniformLocation(ID, name.c_str()), 1, GL_FALSE, &mat[0][0]);
        }
        void setMat4(const std::string& name, const glm::mat4& mat) const
        {
            glUniformMatrix4fv(glGetUniformLocation(ID, name.c_str()), 1, GL_FALSE, &mat[0][0]);
        }

        int get_uniform_binding_location(const std::string& name)
        {
            int loc = glGetUniformLocation(ID, name.c_str());

            if (loc == -1)
            {
                printf("ERROR: Location of uniform not found: %s \n", name.c_str());
                return -1;
            }

            int params[1];
            glGetUniformiv(ID, loc, params);
            return params[0];
        }

        int get_buffer_binding_location(const std::string& name)
        {
            int index = glGetProgramResourceIndex(ID, GL_SHADER_STORAGE_BLOCK, name.c_str());
            int params[1];
            GLenum props[1] = { GL_BUFFER_BINDING };
            glGetProgramResourceiv(ID, GL_SHADER_STORAGE_BLOCK, index, 1, props, 1, NULL, params);
            return params[0];
        }

        void set_compute_buffer_bind_location(const std::string& name)
        {
            int index = glGetProgramResourceIndex(ID, GL_SHADER_STORAGE_BLOCK, name.c_str());

            int location = get_buffer_binding_location(name);

            glShaderStorageBlockBinding(ID, index, location);
        }

        // utility function for checking shader compilation/linking errors.

        void check_compile_errors(GLuint shader, std::string type)
        {
            GLint success;
            GLchar infoLog[1024];
            if (type != "PROGRAM")
            {
                glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
                if (!success)
                {
                    glGetShaderInfoLog(shader, 1024, NULL, infoLog);
                    std::cout << "ERROR::SHADER_COMPILATION_ERROR of type: " << type << "\n" << infoLog << "\n -- --------------------------------------------------- -- " << std::endl;
                    ok_to_build = false;
                }
            }
            else
            {
                glGetProgramiv(shader, GL_LINK_STATUS, &success);
                if (!success)
                {
                    glGetProgramInfoLog(shader, 1024, NULL, infoLog);
                    std::cout << "ERROR::PROGRAM_LINKING_ERROR of type: " << type << "\n"
                        << infoLog << "\n -- --------------------------------------------------- -- " << std::endl;
                    ok_to_build = false;
                }
            }
        }
    };
}
