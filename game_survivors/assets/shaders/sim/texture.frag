// version prepended to file when loaded by engine.
//

out vec4 out_colour;

in SIM_VS_OUT
{
  vec2 v_uv;
} sim_fs_in;

uniform sampler2D tex;
uniform float val;

void
main()
{
  vec2 v_uv = sim_fs_in.v_uv;

  out_colour = val * texture(tex, v_uv);
}