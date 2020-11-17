
//header
#include "engine/mesh/mesh.hpp"

//other library headers
#include <gl/glew.h>
#include <glm/glm.hpp>

//your project headers
#include "engine/graphics/util/opengl_util.hpp"

namespace fightingengine {

Mesh::Mesh(std::vector<glm::vec3> positions, std::vector<unsigned int> indices)
    : Positions(positions)
    , Indices(indices)
{
}

Mesh::Mesh(std::vector<glm::vec3> positions, std::vector<glm::vec2> uv, std::vector<unsigned int> indices)
    : Positions(positions)
    , Indices(indices)
    , UV(uv)
{
}

Mesh::Mesh(std::vector<glm::vec3> positions, std::vector<glm::vec2> uv, std::vector<glm::vec3> normals, std::vector<unsigned int> indices)
    : Positions(positions)
    , Indices(indices)
    , Normals(normals)
    , UV(uv)
{
}

Mesh::Mesh(std::vector<glm::vec3> positions, std::vector<glm::vec2> uv, std::vector<glm::vec3> normals, std::vector<glm::vec3> tangents, std::vector<glm::vec3> bitangents, std::vector<unsigned int> indices)
    : Positions(positions)
    , Indices(indices)
    , Normals(normals)
    , Tangents(tangents)
    , Bitangents(bitangents)
    , UV(uv)
{
}

void Mesh::SetPositions(std::vector<glm::vec3> positions)
{
    Positions = positions;
}

void Mesh::SetUVs(std::vector<glm::vec2> uv)
{
    UV = uv;
}

void Mesh::SetNormals(std::vector<glm::vec3> normals)
{
    Normals = normals;
}

void Mesh::SetTangents(std::vector<glm::vec3> tangents, std::vector<glm::vec3> bitangents)
{
    Tangents = tangents;
    Bitangents = bitangents;
}

void Mesh::Finalize(bool interleaved)
{
    // initialize object IDs if not configured before
    if (!vao)
    {
        CHECK_OPENGL_ERROR(1);
        glGenVertexArrays(1, &vao);
        glGenBuffers(1, &vbo);
        glGenBuffers(1, &ebo);
    }

    // preprocess buffer data as interleaved or seperate when specified
    std::vector<float> data; 
    if (interleaved)
    {
        for (int i = 0; i < Positions.size(); ++i)
        {
            data.push_back(Positions[i].x);
            data.push_back(Positions[i].y);
            data.push_back(Positions[i].z);
            if (UV.size() > 0)
            {
                data.push_back(UV[i].x);
                data.push_back(UV[i].y);
            }
            if (Normals.size() > 0)
            {
                data.push_back(Normals[i].x);
                data.push_back(Normals[i].y);
                data.push_back(Normals[i].z);
            }
            if (Tangents.size() > 0)
            {
                data.push_back(Tangents[i].x);
                data.push_back(Tangents[i].y);
                data.push_back(Tangents[i].z);
            }
            if (Bitangents.size() > 0)
            {
                data.push_back(Bitangents[i].x);
                data.push_back(Bitangents[i].y);
                data.push_back(Bitangents[i].z);
            }
        }
    }
    else
    {
        // if any of the float arrays are empty, data won't be filled by them.
        for (int i = 0; i < Positions.size(); ++i)
        {
            data.push_back(Positions[i].x);
            data.push_back(Positions[i].y);
            data.push_back(Positions[i].z);
        }
        for (int i = 0; i < UV.size(); ++i)
        {
            data.push_back(UV[i].x);
            data.push_back(UV[i].y);
        }
        for (int i = 0; i < Normals.size(); ++i)
        {
            data.push_back(Normals[i].x);
            data.push_back(Normals[i].y);
            data.push_back(Normals[i].z);
        }
        for (int i = 0; i < Tangents.size(); ++i)
        {
            data.push_back(Tangents[i].x);
            data.push_back(Tangents[i].y);
            data.push_back(Tangents[i].z);
        }
        for (int i = 0; i < Bitangents.size(); ++i)
        {
            data.push_back(Bitangents[i].x);
            data.push_back(Bitangents[i].y);
            data.push_back(Bitangents[i].z);
        }
    }

    // configure vertex attributes (only on vertex data size() > 0)
    glBindVertexArray(vao);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, data.size() * sizeof(float), &data[0], GL_STATIC_DRAW);
    // only fill the index buffer if the index array is non-empty.
    if (Indices.size() > 0)
    {
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, Indices.size() * sizeof(unsigned int), &Indices[0], GL_STATIC_DRAW);
    }
    if (interleaved)
    {
        // calculate stride from number of non-empty vertex attribute arrays
        size_t stride                      = 3 * sizeof(float);
        if (UV.size() > 0)         stride += 2 * sizeof(float);
        if (Normals.size() > 0)    stride += 3 * sizeof(float);
        if (Tangents.size() > 0)   stride += 3 * sizeof(float);
        if (Bitangents.size() > 0) stride += 3 * sizeof(float);

        size_t offset = 0;
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, stride, (GLvoid*)offset);
        offset += 3 * sizeof(float);
        if (UV.size() > 0)
        {
            glEnableVertexAttribArray(1);
            glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, stride, (GLvoid*)offset);
            offset += 2 * sizeof(float);
        }
        if (Normals.size() > 0)
        {
            glEnableVertexAttribArray(2);
            glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, stride, (GLvoid*)offset);
            offset += 3 * sizeof(float);
        }
        if (Tangents.size() > 0)
        {
            glEnableVertexAttribArray(3);
            glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, stride, (GLvoid*)offset);
            offset += 3 * sizeof(float);
        }
        if (Bitangents.size() > 0)
        {
            glEnableVertexAttribArray(4);
            glVertexAttribPointer(4, 3, GL_FLOAT, GL_FALSE, stride, (GLvoid*)offset);
            offset += 3 * sizeof(float);
        }
    }
    else
    {
        size_t offset = 0;
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (GLvoid*)offset);
        offset += Positions.size() * sizeof(float);
        if (UV.size() > 0)
        {
            glEnableVertexAttribArray(1);
            glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 0, (GLvoid*)offset);
            offset += UV.size() * sizeof(float);
        }
        if (Normals.size() > 0)
        {
            glEnableVertexAttribArray(2);
            glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 0, (GLvoid*)offset);
            offset += Normals.size() * sizeof(float);
        }
        if (Tangents.size() > 0)
        {
            glEnableVertexAttribArray(3);
            glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, 0, (GLvoid*)offset);
            offset += Tangents.size() * sizeof(float);
        }
        if (Bitangents.size() > 0)
        {
            glEnableVertexAttribArray(4);
            glVertexAttribPointer(4, 3, GL_FLOAT, GL_FALSE, 0, (GLvoid*)offset);
            offset += Bitangents.size() * sizeof(float);
        }
    }
    glBindVertexArray(0);
}


// void Mesh::draw(Shader& shader)
// {
//     // if (texture == -1)
//     // {
//     //     // bind appropriate textures
//     //     unsigned int diffuseNr = 1;
//     //     unsigned int specularNr = 1;
//     //     unsigned int normalNr = 1;
//     //     unsigned int heightNr = 1;
//     //     for (unsigned int i = 0; i < textures.size(); i++)
//     //     {
//     //         glActiveTexture(GL_TEXTURE0 + i); // active proper texture unit before binding
//     //         // retrieve texture number (the N in diffuse_textureN)
//     //         std::string number;
//     //         std::string name = textures[i].type;
//     //         if (name == "texture_diffuse")
//     //             number = std::to_string(diffuseNr++);
//     //         else if (name == "texture_specular")
//     //             number = std::to_string(specularNr++); // transfer unsigned int to stream
//     //         else if (name == "texture_normal")
//     //             number = std::to_string(normalNr++); // transfer unsigned int to stream
//     //         else if (name == "texture_height")
//     //             number = std::to_string(heightNr++); // transfer unsigned int to stream
//     //         // now set the sampler to the correct texture unit
//     //         const char* tex_name = (name + number).c_str();
//     //         shader.setInt(tex_name, i);
//     //         glBindTexture(GL_TEXTURE_2D, textures[i].id);
//     //     }
//     // }
//     // else
//     // {
//     //     //glActiveTexture(GL_TEXTURE0);
//     //     glBindTexture(GL_TEXTURE_2D, texture);
//     //     //printf("texture %i", texture);
//     //     shader.setInt("texture_diffuse1", texture);
//     // }
//     // draw mesh
//     glBindVertexArray(VAO);
//     glDrawElements(GL_TRIANGLES, indices.size(), GL_UNSIGNED_INT, 0);
//     glBindVertexArray(0);
//     // always good practice to set everything back to defaults once configured.
//     glActiveTexture(GL_TEXTURE0);
// }

} //namespace fightingengine
