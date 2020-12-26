
// header
#include "engine/graphics/framebuffer.hpp"

// other lib headers
#include <GL/glew.h>
#include <spdlog/spdlog.h>

namespace fightingengine {

unsigned int
Framebuffer::create_fbo()
{
  unsigned int fbo;
  glGenFramebuffers(1, &fbo);
  return fbo;
}

void
Framebuffer::bind_fbo(unsigned int fbo)
{
  glBindFramebuffer(GL_FRAMEBUFFER, fbo);
}

void
Framebuffer::default_fbo()
{
  glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void
Framebuffer::fbo_enable_writing(unsigned int fbo)
{
  glBindFramebuffer(GL_DRAW_FRAMEBUFFER, fbo);
}

void
Framebuffer::fbo_disable_writing(unsigned int fbo)
{
  glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
}

unsigned int
Framebuffer::create_rbo()
{
  unsigned int rbo;
  glGenRenderbuffers(1, &rbo);
  return rbo;
}

void
Framebuffer::bind_rbo(unsigned int rbo)
{
  glBindRenderbuffer(GL_RENDERBUFFER, rbo);
}

void
Framebuffer::unbind_rbo()
{
  glBindRenderbuffer(GL_RENDERBUFFER, 0);
}

} // fightingengine