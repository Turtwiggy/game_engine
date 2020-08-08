
#include "3d/fg_texture.hpp"

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

namespace fightingengine {

    unsigned int TextureFromFile(const char* file, const std::string& directory, bool gamma)
    {
        std::string filename = std::string(file);
        filename = directory + '/' + filename;

        printf("----- Texture from path -------\n");
        printf("Dir: %s \n", directory.c_str());
        printf("Path: %s \n", file);
        printf("Filename: %s \n", filename.c_str());
        printf("----- End Texture -------\n");

        unsigned int textureID;
        glGenTextures(1, &textureID);

        int width, height, nrComponents;
        unsigned char* data = stbi_load(filename.c_str(), &width, &height, &nrComponents, 0);

        if (data)
        {
            GLenum internal_format, data_format;

            if (nrComponents == 3)
            {
                internal_format = gamma ? GL_SRGB8 : GL_RGB8;
                data_format = gamma ? GL_SRGB : GL_RGB;
            }
            else if (nrComponents == 4)
            {
                internal_format = gamma ? GL_SRGB8_ALPHA8 : GL_RGBA8;
                data_format = gamma ? GL_SRGB_ALPHA : GL_RGBA;
            }

            //note, if this throws an error opengl is not init
            //glCreateTextures(GL_TEXTURE_2D, 1, &textureID);
            //glTextureStorage2D(textureID, 1, internal_format, width, height);

            //glTextureParameteri(textureID, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
            //glTextureParameteri(textureID, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

            //glTextureParameteri(textureID, GL_TEXTURE_WRAP_S, GL_REPEAT);
            //glTextureParameteri(textureID, GL_TEXTURE_WRAP_T, GL_REPEAT);

            //glTextureSubImage2D(textureID, 0, 0, 0, width, height, data_format, GL_UNSIGNED_BYTE, data);

            glBindTexture(GL_TEXTURE_2D, textureID);
            glTexImage2D(GL_TEXTURE_2D, 0, data_format, width, height, 0, data_format, GL_UNSIGNED_BYTE, data);
            glGenerateMipmap(GL_TEXTURE_2D);

            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        }
        else
        {
            printf("Texture failed to load at path: %s \n", file);
        }
        stbi_image_free(data);

        return textureID;
    }

}
