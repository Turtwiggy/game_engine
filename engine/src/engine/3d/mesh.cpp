
//header
#include "engine/3d/mesh.hpp"

//other library headers
#include <gl/glew.h>
// #include <glm/glm.hpp>
// #include <assimp/Importer.hpp>
// #include <assimp/scene.h>
// #include <assimp/postprocess.h>

//your project headers
#include "engine/graphics/triangle.hpp"

namespace fightingengine {

Mesh::Mesh
(
    std::string               name,
    std::vector<Vertex>       vertices,
    std::vector<unsigned int> indices,
    std::vector<Texture2D>    textures,
    ColourVec4f               colour
)
    : vertices(vertices)
    , indices(indices)
    , textures(textures)
    , colour(colour)
    , name(name)
{
    printf("setting up mesh %s \n", name.c_str());
    setup_mesh();
}

void Mesh::draw(Shader& shader)
{
    // if (texture == -1)
    // {
    //     // bind appropriate textures
    //     unsigned int diffuseNr = 1;
    //     unsigned int specularNr = 1;
    //     unsigned int normalNr = 1;
    //     unsigned int heightNr = 1;
    //     for (unsigned int i = 0; i < textures.size(); i++)
    //     {
    //         glActiveTexture(GL_TEXTURE0 + i); // active proper texture unit before binding
    //         // retrieve texture number (the N in diffuse_textureN)
    //         std::string number;
    //         std::string name = textures[i].type;
    //         if (name == "texture_diffuse")
    //             number = std::to_string(diffuseNr++);
    //         else if (name == "texture_specular")
    //             number = std::to_string(specularNr++); // transfer unsigned int to stream
    //         else if (name == "texture_normal")
    //             number = std::to_string(normalNr++); // transfer unsigned int to stream
    //         else if (name == "texture_height")
    //             number = std::to_string(heightNr++); // transfer unsigned int to stream

    //         // now set the sampler to the correct texture unit
    //         const char* tex_name = (name + number).c_str();
    //         shader.setInt(tex_name, i);
    //         glBindTexture(GL_TEXTURE_2D, textures[i].id);
    //     }
    // }
    // else
    // {
    //     //glActiveTexture(GL_TEXTURE0);
    //     glBindTexture(GL_TEXTURE_2D, texture);
    //     //printf("texture %i", texture);
    //     shader.setInt("texture_diffuse1", texture);
    // }

    // draw mesh
    glBindVertexArray(VAO);
    glDrawElements(GL_TRIANGLES, indices.size(), GL_UNSIGNED_INT, 0);
    glBindVertexArray(0);

    // always good practice to set everything back to defaults once configured.
    glActiveTexture(GL_TEXTURE0);
}

void Mesh::setup_mesh()
{
    // //generate triangle information
    // if (indices.size() % 3 != 0)
    // {
    //     printf("(Mesh) setup_mesh: got a triangle problem scotty... \n");
    // }
    // else
    // {
    //     printf("(Mesh) generating triangle information \n");
    //     //generate triangle information
    //     for (int i = 0; i < indices.size(); i += 3)
    //     {
    //         Triangle tri;
    //         tri.p0 = vertices[indices[i]];
    //         tri.p0.Colour = colour;
    //         tri.p1 = vertices[indices[i + 1]];
    //         tri.p1.Colour = colour;
    //         tri.p2 = vertices[indices[i + 2]];
    //         tri.p2.Colour = colour;
    //         triangles.push_back(tri);
    //     }
    // }
    // printf("mesh: %s made of %i triangles ", name.c_str(), (int)triangles.size());

    // create buffers/arrays
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glGenBuffers(1, &EBO);

    //fill buffers
    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(Vertex), &vertices[0], GL_STATIC_DRAW);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), &indices[0], GL_STATIC_DRAW);

    //link vertex attributes
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)0);
    // vertex normals
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, Normal));
    // vertex texture coords
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, TexCoords));
    //// vertex tangent
    //glEnableVertexAttribArray(3);
    //glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, sizeof(FGVertex), (void*)offsetof(FGVertex, Tangent));
    //// vertex bitangent
    //glEnableVertexAttribArray(4);
    //glVertexAttribPointer(4, 3, GL_FLOAT, GL_FALSE, sizeof(FGVertex), (void*)offsetof(FGVertex, Bitangent));
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
}


} //namespace fightingengine
