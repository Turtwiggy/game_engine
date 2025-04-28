// version prepended to file when loaded by engine.
//

out vec4 out_colour;

in SIM_VS_OUT
{
  vec2 v_uv;
} sim_fs_in;

uniform vec2 texel_size;
uniform sampler2D u_pressure;
uniform sampler2D u_divergence;

void
main()
{
  vec2 v_uv = sim_fs_in.v_uv;

  vec2 vL = v_uv - vec2(texel_size.x, 0.0);
  vec2 vR = v_uv + vec2(texel_size.x, 0.0);
  vec2 vT = v_uv + vec2(0.0, texel_size.y);
  vec2 vB = v_uv - vec2(0.0, texel_size.y);

  float L = texture2D(u_pressure, vL).x;
  float R = texture2D(u_pressure, vR).x;
  float T = texture2D(u_pressure, vT).x;
  float B = texture2D(u_pressure, vB).x;
  float C = texture2D(u_pressure, v_uv).x;

  float divergence = texture2D(u_divergence, v_uv).r;
  float pressure = (L + R + B + T - divergence) * 0.25;

  // pressure *= 0.25f;

  out_colour = vec4(pressure, 0.0, 0.0, 1.0);
}