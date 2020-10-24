#pragma once

//c++ standard library headers
#include <string>

namespace fightingengine {

struct Texture2D {
    unsigned int id;
    unsigned int width, height; // width and height of loaded image in pixels
    // texture Format
    unsigned int Internal_Format; // format of texture object
    unsigned int Image_Format; // format of loaded image
    // texture configuration
    unsigned int Wrap_S; // wrapping mode on S axis
    unsigned int Wrap_T; // wrapping mode on T axis
    unsigned int Filter_Min; // filtering mode if texture pixels < screen pixels
    unsigned int Filter_Max; // filtering mode if texture pixels > screen pixels
    std::string path;
    std::string type; //diffuse, specular, normal, height

    Texture2D() = default;
    Texture2D(std::string path);

    // generates texture from image data
    void Generate(unsigned int width, unsigned int height, unsigned char* data);
    // binds the texture as the current active GL_TEXTURE_2D texture object
    void Bind() const;
};

} //namespace fightingengine
