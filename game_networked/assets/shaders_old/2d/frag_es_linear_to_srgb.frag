#version 110

precision mediump float;

varying vec2 v_tex;
varying vec4 v_colour; 
varying vec2 v_sprite_pos;
varying vec2 v_sprites;
varying float v_tex_unit;

uniform sampler2D textures[1];

float
linear_to_srgb(float f)
{
  if (f <= 0.0031308)
    return 12.92 * f;
  return 1.055 * pow(f, 1.0 / 2.4) - 0.055;
}

void
main()
{
  vec4 lin = texture2D(textures[0], v_tex);

  vec3 rgb = vec3(
    linear_to_srgb(lin.r),
    linear_to_srgb(lin.g),
    linear_to_srgb(lin.b)
  );

  gl_FragColor = vec4(
    rgb.r,
    rgb.g,
    rgb.b,
    lin.a
  );
}
