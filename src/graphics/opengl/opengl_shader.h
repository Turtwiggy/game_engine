#pragma once

#ifndef SHADER_H
#define SHADER_H

#include <glm/glm.hpp>
#include <GL/glew.h>

#include <vector>
#include <string>
#include <fstream>
#include <sstream>
#include <iostream>

class Shader
{
public:
    Shader() = default;

    unsigned int ID;
    // constructor generates the shader on the fly
    // ------------------------------------------------------------------------
    Shader(const char* vertexPath, const char* fragmentPath, const char* geometryPath = nullptr)
    {
        std::cout
            << "vertex path: " << vertexPath << "\n"
            << "frag path: " << fragmentPath << "\n" << std::endl;

        std::vector<unsigned int> shaders;

        unsigned int vert_id = load_shader(vertexPath, GL_VERTEX_SHADER, "VERTEX");
        shaders.push_back(vert_id);

        unsigned int frag_id = load_shader(fragmentPath, GL_FRAGMENT_SHADER, "FRAGMENT");
        shaders.push_back(frag_id);

        unsigned int geom_id;
        if (geometryPath != nullptr)
        {
            geom_id; load_shader(geometryPath, GL_GEOMETRY_SHADER, "GEOMETRY");
            shaders.push_back(geom_id);
        }

        attach_shaders_to_program(shaders);
    }

    Shader(const char* computePath)
    {
        std::cout << "compute path: " << computePath << "\n" << std::endl;

        std::vector<unsigned int> shaders;

        unsigned int compute_id = load_shader(computePath, GL_COMPUTE_SHADER, "COMPUTE");
        shaders.push_back(compute_id);

        attach_shaders_to_program(shaders);
    }

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
            }
        }

        const char* csCode = code.c_str();
        shader_id = glCreateShader(gl_shader_type);
        glShaderSource(shader_id, 1, &csCode, NULL);
        glCompileShader(shader_id);
        Shader::check_compile_errors(shader_id, type);

        return shader_id;
    }

    void attach_shaders_to_program(std::vector<unsigned int>& shaders)
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


    // activate the shader
    // ------------------------------------------------------------------------
    void use()
    {
        glUseProgram(ID);
    }
    // utility uniform functions
    // ------------------------------------------------------------------------
    void setBool(const std::string& name, bool value) const
    {
        glUniform1i(glGetUniformLocation(ID, name.c_str()), (int)value);
    }
    // ------------------------------------------------------------------------
    void setInt(const std::string& name, int value) const
    {
        glUniform1i(glGetUniformLocation(ID, name.c_str()), value);
    }
    // ------------------------------------------------------------------------
    void setFloat(const std::string& name, float value) const
    {
        glUniform1f(glGetUniformLocation(ID, name.c_str()), value);
    }
    // ------------------------------------------------------------------------
    void setVec2(const std::string& name, const glm::vec2& value) const
    {
        glUniform2fv(glGetUniformLocation(ID, name.c_str()), 1, &value[0]);
    }
    void setVec2(const std::string& name, float x, float y) const
    {
        glUniform2f(glGetUniformLocation(ID, name.c_str()), x, y);
    }
    // ------------------------------------------------------------------------
    void setVec3(const std::string& name, const glm::vec3& value) const
    {
        glUniform3fv(glGetUniformLocation(ID, name.c_str()), 1, &value[0]);
    }
    void setVec3(const std::string& name, float x, float y, float z) const
    {
        glUniform3f(glGetUniformLocation(ID, name.c_str()), x, y, z);
    }
    // ------------------------------------------------------------------------
    void setVec4(const std::string& name, const glm::vec4& value) const
    {
        glUniform4fv(glGetUniformLocation(ID, name.c_str()), 1, &value[0]);
    }
    void setVec4(const std::string& name, float x, float y, float z, float w)
    {
        glUniform4f(glGetUniformLocation(ID, name.c_str()), x, y, z, w);
    }
    // ------------------------------------------------------------------------
    void setMat2(const std::string& name, const glm::mat2& mat) const
    {
        glUniformMatrix2fv(glGetUniformLocation(ID, name.c_str()), 1, GL_FALSE, &mat[0][0]);
    }
    // ------------------------------------------------------------------------
    void setMat3(const std::string& name, const glm::mat3& mat) const
    {
        glUniformMatrix3fv(glGetUniformLocation(ID, name.c_str()), 1, GL_FALSE, &mat[0][0]);
    }
    // ------------------------------------------------------------------------
    void setMat4(const std::string& name, const glm::mat4& mat) const
    {
        glUniformMatrix4fv(glGetUniformLocation(ID, name.c_str()), 1, GL_FALSE, &mat[0][0]);
    }

    // utility function for checking shader compilation/linking errors.
    // ------------------------------------------------------------------------
    static void check_compile_errors(GLuint shader, std::string type)
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
            }
        }
    }
};
#endif
