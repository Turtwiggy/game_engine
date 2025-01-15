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

void
main()
{
  // out_colour = vec4(v_uv.x, v_uv.y, 0.0, 1.0);
  out_colour = vec4(1.0, 0.0, 0.0, 1.0);
}
