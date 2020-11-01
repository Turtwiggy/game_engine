#pragma once

//other library headers
#include <GL/glew.h>

namespace fightingengine {

//A "TextureCube" or "Skybox" is 6 textures.
 
class TextureCube
{
public:
    unsigned int ID;
    GLenum InternalFormat = GL_RGBA;            // the format each texel is stored in
    GLenum Format         = GL_RGBA;            // the format each texel is stored in
    GLenum Type           = GL_UNSIGNED_BYTE;
    GLenum FilterMin      = GL_LINEAR;          // what filter method to use during minification
    GLenum FilterMax      = GL_LINEAR;          // what filter method to use during magnification
    GLenum WrapS          = GL_CLAMP_TO_EDGE;   // wrapping method of the S coordinate
    GLenum WrapT          = GL_CLAMP_TO_EDGE;   // wrapping method of the T coordinate
    GLenum WrapR          = GL_CLAMP_TO_EDGE;   // wrapping method of the T coordinate
    bool   Mipmapping     = false;
    
    unsigned int FaceWidth  = 0;
    unsigned int FaceHeight = 0;

public:
        TextureCube() = default;
        ~TextureCube() = default;

        //Note, instead of using the raw functions here, it is easier to use the 
        //ResourceManager::load_texture_cube() function

        // default initialize all cubemap faces w/ default values
        void DefaultInitialize(
            unsigned int width, 
            unsigned int height, 
            GLenum format, 
            GLenum type, 
            bool mipmap = false );

        // cubemap texture generation per face
        void GenerateFace(
            GLenum face, 
            unsigned int width, 
            unsigned int height, 
            GLenum format, 
            GLenum type, 
            unsigned char* data );
        
        void Bind(int slot = -1);
        void Unbind();
};

} //namespace fightingengine