// version prepended to file when loaded by engine.
//

out vec4 out_colour;

in SIM_VS_OUT
{
  vec2 v_uv;
} sim_fs_in;

uniform sampler2D u_velocity;
uniform sampler2D u_curl;
uniform vec2 texel_size;
uniform float dt;
uniform float curl;

void
main()
{
  vec2 v_uv = sim_fs_in.v_uv;

  vec2 vL = v_uv - vec2(texel_size.x, 0.0);
  vec2 vR = v_uv + vec2(texel_size.x, 0.0);
  vec2 vT = v_uv + vec2(0.0, texel_size.y);
  vec2 vB = v_uv - vec2(0.0, texel_size.y);

  float L = texture(u_curl, vL).x;
  float R = texture(u_curl, vR).x;
  float T = texture(u_curl, vT).x;
  float B = texture(u_curl, vB).x;
  float C = texture(u_curl, v_uv).x;

  vec2 force = 0.5 * vec2(abs(T) - abs(B), abs(R) - abs(L));
  force /= length(force) + 0.0001;
  force *= curl * C;
  force.y *= -1.0;

  vec2 velocity = texture(u_velocity, v_uv).xy;
  velocity += force * dt;
  velocity = min(max(velocity, -1000.0), 1000.0);

  out_colour = vec4(velocity, 0.0, 1.0);
}