#version 110

precision mediump float;

uniform vec4 colour;

void
main()
{
  gl_FragColor = colour;
}