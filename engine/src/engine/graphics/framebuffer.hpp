#pragma onces

//c++ lib headers
#include <array>

//your project files
#include "engine/graphics/texture.hpp"

namespace fightingengine {

class Framebuffer
{
public:
    static unsigned int create_fbo();
    static void bind_fbo(unsigned int fbo);
    static void default_fbo();

    static void fbo_enable_writing(unsigned int fbo);
    static void fbo_disable_writing(unsigned int fbo); 

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
    // static unsigned int create_picking_fbo(int width, int height, Texture2D& colour_tex, Texture2D& depth_tex);
    // static void create_picking_colourbuffer_texture(Texture2D& tex, int tex_width, int tex_height);
    // static void create_picking_depthbuffer_texture(Texture2D& tex, int tex_width, int tex_height);


    // --- shadow mapping fbo usage
public:
    static void create_shadowmap_depthbuffer( int fbo, Texture2D& tex, int tex_width, int tex_height );
};

} //namespace fightingengine