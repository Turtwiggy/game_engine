// version prepended to file when loaded by engine.
//

out vec4 out_colour;

in SIM_VS_OUT
{
  vec2 v_uv;
} sim_fs_in;

uniform sampler2D u_velocity;
uniform vec2 texel_size;

void
main()
{
  vec2 v_uv = sim_fs_in.v_uv;

  vec2 vL = v_uv - vec2(texel_size.x, 0.0);
  vec2 vR = v_uv + vec2(texel_size.x, 0.0);
  vec2 vT = v_uv + vec2(0.0, texel_size.y);
  vec2 vB = v_uv - vec2(0.0, texel_size.y);

  float L = texture(u_velocity, vL).x;
  float R = texture(u_velocity, vR).x;
  float T = texture(u_velocity, vT).y;
  float B = texture(u_velocity, vB).y;

  vec2 C = texture(u_velocity, v_uv).xy;
  if (vL.x < 0.0) { L = -C.x; }
  if (vR.x > 1.0) { R = -C.x; }
  if (vT.y > 1.0) { T = -C.y; }
  if (vB.y < 0.0) { B = -C.y; }

  float div = 0.5 * (R - L + T - B);
  
  out_colour = vec4(div, 0.0, 0.0, 1.0);
}