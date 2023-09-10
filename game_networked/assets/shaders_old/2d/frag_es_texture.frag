#version 110

precision mediump float;

varying vec2 v_tex;

uniform sampler2D textures[1];

void
main()
{
  // color = sprite_colour; //* texture(tex, TexCoords);
  // color = sprite_colour * texture(tex, TexCoords);
  // color = vec4(1.0, 0.0, 1.0, 1.0);
  gl_FragColor = texture(textures[0], v_tex);
}