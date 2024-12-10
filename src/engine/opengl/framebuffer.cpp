
// header
#include "engine/opengl/framebuffer.hpp"

// other lib headers
#include "engine/deps/opengl.hpp"

namespace engine {

FramebufferID
Framebuffer::create_fbo()
{
  unsigned int fbo;
  glGenFramebuffers(1, &fbo);
  FramebufferID fbo_id;
  fbo_id.id = fbo;
  return fbo_id;
}

void
Framebuffer::bind_fbo(const FramebufferID& fbo)
{
  glBindFramebuffer(GL_FRAMEBUFFER, fbo.id);
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