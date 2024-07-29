#version 150


out vec4 out_colour;

in vec2 v_uv;
in vec4 v_colour; 
in vec2 v_sprite_pos;
in vec2 v_sprites;
in float v_tex_unit;

uniform sampler2D tex;

float
linear_to_srgb(float f)
{
  if (f <= 0.0031308f)
    return 12.92f * f;
  return 1.055f * pow(f, 1.0f / 2.4f) - 0.055f;
}

void
main()
{
  vec4 lin = texture(tex, v_uv);

  // vec3 rgb = vec3(
  //   linear_to_srgb(lin.r),
  //   linear_to_srgb(lin.g),
  //   linear_to_srgb(lin.b)
  // );

  out_colour = vec4(
    lin.r,
    lin.g,
    lin.b,
    lin.a
  );

}
