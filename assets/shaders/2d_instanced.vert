#version 330 core

layout(location = 0) in vec3 pos;
layout(location = 1) in vec4 colour;

// out vec2 v_tex;
out vec4 v_colour;

uniform mat4 model;
// note that we're omitting the view matrix; the view never changes so we
// basically have an identity view matrix and can therefore omit it.
uniform mat4 projection;

void
main()
{
  // v_tex = vertex.zw;
  v_colour = colour;

  gl_Position = vec4(pos.xyz, 1.0);
}