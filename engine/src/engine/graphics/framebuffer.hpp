#pragma once

//c++ lib headers
#include <array>

//your project files
#include "engine/graphics/texture.hpp"

namespace fightingengine {

struct PixelInfo
{
    float object_id = 0.0f;
    float draw_id = 0.0f;
    float prim_id = 0.0f;
};

class Framebuffer
{
public:
    static unsigned int create_fbo();
    static void bind_fbo(unsigned int fbo);
    static void unbind_fbo();

    static void fbo_enable_writing(unsigned int fbo);
    static void fbo_disable_writing(unsigned int fbo); 

    //reads a pixel from an fbo's GL_COLOR_ATTACHMENT0 using glReadPixels
    //note: for the moment, only reads GL_RGB data
    static PixelInfo read_fbo_pixel(unsigned int fbo, int x, int y);

    //Renderbuffer objects store all the render data directly into
    //their buffer without any conversions to texture-specific formats,
    //making them faster as a writeable storage medium. You cannot read
    //from them directly, but it is possible to read from them via the
    //**slow** glReadPixels. This returns a specified area of pixels from the
    //currently bound framebuffer, but not directly from the attachment itself.
    static unsigned int create_rbo(); 
    static void bind_rbo(unsigned int rbo);
    static void unbind_rbo();

    // ---- object picking specific fbo usage

public:
    static unsigned int create_picking_fbo(int width, int height, Texture2D& colour_tex, Texture2D& depth_tex);

private:
    //note: bind fbo before calling this
    static void create_picking_colourbuffer_texture(Texture2D& tex, int tex_width, int tex_height);

    //note: bind fbo before calling this
    static void create_picking_depthbuffer_texture(Texture2D& tex, int tex_width, int tex_height);

};

} //namespace fightingengine