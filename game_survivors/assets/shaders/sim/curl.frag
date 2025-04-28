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

  float L = texture2D(u_velocity, vL).x;
  float R = texture2D(u_velocity, vR).x;
  float T = texture2D(u_velocity, vT).x;
  float B = texture2D(u_velocity, vB).x;
  float vorticity = 0.5 * ( R - L - T + B );

  out_colour = vec4(vorticity, 0.0, 0.0, 1.0);
}