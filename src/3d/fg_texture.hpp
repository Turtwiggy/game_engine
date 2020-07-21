#pragma once

#include "GL/glew.h"

#include <string>
#include <stb_image.h>

namespace fightinggame {

    struct Texture2D {
        unsigned int id;
        std::string type;
        std::string path;
    };

    unsigned int TextureFromFile(const char* path, const std::string& directory, bool gamma)
    {
        std::cout << "----- Texture from path -------" << std::endl;
        //  std::cout << "Filename: " << file_name << std::endl;
        std::cout << "Dir: " << directory << std::endl;
        std::cout << "Path: " << path << std::endl;
        std::cout << "----- End Texture -------" << std::endl;

        std::string filename = std::string(path);
        filename = directory + '/' + filename;

        unsigned int textureID;
        glGenTextures(1, &textureID);

        int width, height, nrComponents;
        unsigned char* data = stbi_load(filename.c_str(), &width, &height, &nrComponents, 0);
        if (data)
        {
            GLenum format;
            if (nrComponents == 1)
                format = GL_RED;
            else if (nrComponents == 3)
                format = GL_RGB;
            else if (nrComponents == 4)
                format = GL_RGBA;

           //note, if this throws an error opengl is not init
            glBindTexture(GL_TEXTURE_2D, textureID);
            glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
            glGenerateMipmap(GL_TEXTURE_2D);

            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        }
        else
        {
            std::cout << "Texture failed to load at path: " << path << std::endl;
        }
        stbi_image_free(data);

        return textureID;
    }

}
