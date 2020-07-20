#pragma once

#include "graphics/texture.h"
#include "graphics/opengl/opengl_texture.h"
#include "3d/fg_mesh.hpp"
#include "3d/fg_model.hpp"

#include <glm/glm.hpp>

#include <vector>

namespace fightinggame {

    class Terrain
    {

    private:
        const float SIZE = 5.0;
        const int VERTEX_COUNT = 16;

        std::vector<std::shared_ptr<Texture2D>> textures;
        std::shared_ptr<FGMesh> mesh;

        float x;
        float z;

    public:

        Terrain(int grid_x, int grid_z, std::vector<std::shared_ptr<Texture2D>> textures)
            : x(grid_x * SIZE)
            , z(grid_z * SIZE)
            , textures(textures)
        {
            mesh = std::make_shared<FGMesh>(generate_terrain());
        }

        std::shared_ptr<FGMesh> get_mesh()
        {
            return mesh;
        }

    private:

        FGMesh generate_terrain()
        {
            const int count = VERTEX_COUNT * VERTEX_COUNT;
            std::vector<FGVertex> verts;
            verts.resize(count);

            std::vector<unsigned int> indices;
            indices.resize(6.0 * (VERTEX_COUNT - 1.0) * (VERTEX_COUNT - 1.0));

            int counter = 0;
            for (int i = 0; i < VERTEX_COUNT; i++)
            {
                for (int j = 0; j < VERTEX_COUNT; j++)
                {
                    FGVertex vertex;

                    vertex.Position.x = -(float)j / ((float)VERTEX_COUNT - 1) * SIZE;
                    vertex.Position.y = 0.0f;
                    vertex.Position.z = -(float)i / ((float)VERTEX_COUNT - 1) * SIZE;

                    vertex.Normal.x = 0.0f;
                    vertex.Normal.y = 1.0f;
                    vertex.Normal.z = 0.0f;

                    vertex.TexCoords.x = (float)j / ((float)VERTEX_COUNT - 1);
                    vertex.TexCoords.x = (float)i / ((float)VERTEX_COUNT - 1);

                    verts[counter] = vertex;
                    counter += 1;
                }
            }

            int pointer = 0;
            for (int gz = 0; gz < VERTEX_COUNT-1; gz++)
            {
                for (int gx = 0; gx < VERTEX_COUNT-1; gx++)
                {
                    int top_left = (gz * VERTEX_COUNT) + gx;
                    int top_right = top_left + 1;
                    int bottom_left = ((gz + 1) * VERTEX_COUNT) + gx;
                    int bottom_right = bottom_left + 1;

                    indices[pointer++] = top_left;
                    indices[pointer++] = bottom_left;
                    indices[pointer++] = top_right;
                    indices[pointer++] = top_right;
                    indices[pointer++] = bottom_left;
                    indices[pointer++] = bottom_right;
 
                }
            }

            return FGMesh(verts, indices, textures, "Generated Terrain");
        }


    };

}
