
//header
#include "engine/graphics/framebuffer.hpp"

//other lib headers
#include <spdlog/spdlog.h>
#include <GL/glew.h>

namespace fightingengine {

unsigned int Framebuffer::create_fbo()
{
    unsigned int fbo;
    glGenFramebuffers(1, &fbo);
    return fbo; //returns a copy of the unsigned int
}

void Framebuffer::bind_fbo(unsigned int fbo)
{
    glBindFramebuffer(GL_FRAMEBUFFER, fbo);
}

void Framebuffer::unbind_fbo()
{
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void Framebuffer::fbo_enable_writing(unsigned int fbo)
{
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, fbo);
}

void Framebuffer::fbo_disable_writing(unsigned int fbo)
{
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
}

PixelInfo Framebuffer::read_fbo_pixel(unsigned int fbo, int x, int y)
{
    //contains object_id, draw_id, prim_id
    PixelInfo pixel;

    bind_fbo(fbo);
    
        glReadBuffer(GL_COLOR_ATTACHMENT0);

        glReadPixels(x, y, 1, 1, GL_RGB, GL_FLOAT, &pixel);

        glReadBuffer(GL_NONE);
    
    unbind_fbo();

    return pixel; //returns a copy of pixel
}


unsigned int Framebuffer::create_rbo()
{
    unsigned int rbo;
    glGenRenderbuffers(1, &rbo);
    return rbo;
}

void Framebuffer::bind_rbo(unsigned int rbo)
{
    glBindRenderbuffer(GL_RENDERBUFFER, rbo);
}

void Framebuffer::unbind_rbo()
{
    glBindRenderbuffer(GL_RENDERBUFFER, 0);
}


unsigned int Framebuffer::create_picking_fbo(int width, int height, Texture2D& colour_tex, Texture2D& depth_tex)
{
    unsigned int fbo = create_fbo();
    bind_fbo(fbo);
    {
        create_picking_colourbuffer_texture(colour_tex, width, height);
        create_picking_depthbuffer_texture(depth_tex, width, height);
        
        // Disable reading to avoid problems with older GPUs
        glReadBuffer(GL_NONE);

        // tell OpenGL which color attachments we'll 
        // use (of this framebuffer) for rendering 
        glDrawBuffer(GL_COLOR_ATTACHMENT0);

        // finally check if framebuffer is complete
        if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
            spdlog::error("Framebuffer not complete");
    }
    // Restore the default framebuffer
    glBindTexture(GL_TEXTURE_2D, 0);
    unbind_fbo();

    return fbo;
}

void Framebuffer::create_picking_colourbuffer_texture(Texture2D& tex, int tex_width, int tex_height)
{
    tex.FilterMin = GL_LINEAR;
    tex.FilterMax = GL_LINEAR;
    tex.WrapS = GL_CLAMP_TO_EDGE;
    tex.WrapT = GL_CLAMP_TO_EDGE;
    tex.Generate(tex_width, tex_height, GL_RGB32F, GL_RGB, GL_FLOAT, 0);

    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, tex.id, 0);
}

void Framebuffer::create_picking_depthbuffer_texture(Texture2D& tex, int tex_width, int tex_height)
{
    tex.FilterMin = GL_LINEAR;
    tex.FilterMax = GL_LINEAR;
    tex.WrapS = GL_CLAMP_TO_EDGE;
    tex.WrapT = GL_CLAMP_TO_EDGE;
    tex.Generate(tex_width, tex_height, GL_DEPTH_COMPONENT, GL_DEPTH_COMPONENT, GL_FLOAT, 0);

    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, tex.id, 0);
}

} //fightingengine