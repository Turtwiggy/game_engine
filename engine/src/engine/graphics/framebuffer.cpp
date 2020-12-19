
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
    return fbo;
}

void Framebuffer::bind_fbo(unsigned int fbo)
{
    glBindFramebuffer(GL_FRAMEBUFFER, fbo);
}

void Framebuffer::default_fbo()
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

void Framebuffer::create_shadowmap_depthbuffer( 
    unsigned int& depth_map_fbo, 
    unsigned int& depth_map, 
    int tex_width, 
    int tex_height )
{
    glGenFramebuffers(1, &depth_map_fbo);
    // create depth texture
    glGenTextures(1, &depth_map);
    glBindTexture(GL_TEXTURE_2D, depth_map);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, tex_width, tex_height, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
    float borderColor[] = { 1.0, 1.0, 1.0, 1.0 };
    glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, borderColor);
    // attach depth texture as FBO's depth buffer
    glBindFramebuffer(GL_FRAMEBUFFER, depth_map_fbo);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, depth_map, 0);
    glDrawBuffer(GL_NONE);
    glReadBuffer(GL_NONE);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

} //fightingengine