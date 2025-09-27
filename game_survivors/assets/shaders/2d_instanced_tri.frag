// version prepended to file when loaded by engine.
//

out vec4 out_colour;

in VS_OUT
{
  vec2 v_uv;
  vec4 v_colour;
  vec2 v_vertex;
} fs_in;

void
main()
{
  vec2 v_uv = fs_in.v_uv;
  vec4 v_colour = fs_in.v_colour;

  // note: v_colour is in linear. and this outputs linear.
  out_colour = v_colour;
  // out_colour = vec4(1.0, 0.0, 0.0, 1.0);
}