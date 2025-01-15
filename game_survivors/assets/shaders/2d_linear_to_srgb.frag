// version prepended to file when loaded by engine.
//

out vec4 out_colour;

in VS_OUT
{
  vec2 v_uv;
  vec4 v_colour;
  vec2 v_sprite_pos;  // x, y location of sprite
  vec2 v_sprite_wh;   // desired sprites e.g. 2, 2
  vec2 v_sprite_max;  // 22 sprites
  float v_tex_unit;
  vec2 v_vertex;
} fs_in;

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
  vec2 v_uv = fs_in.v_uv;
  
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
