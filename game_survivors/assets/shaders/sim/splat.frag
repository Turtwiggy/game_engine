// version prepended to file when loaded by engine.
//

out vec4 out_colour;

in SIM_VS_OUT
{
  vec2 v_uv;
} sim_fs_in;

// uniform float aspect_ratio;
uniform sampler2D u_target;
uniform vec3 colour;
uniform vec2 point; // in the range [0, 1]
uniform float radius;

void
main()
{
  // v_uv is in the range [0, 1]
  vec2 v_uv = sim_fs_in.v_uv;

  vec3 cur_vel = texture2D(u_target, v_uv).xyz;
  vec2 p = v_uv - point;

  // update texture with splat (velocity and dye textures)
  float gauss = exp(-dot(p, p) / radius);
  vec3 inj_vel = gauss * colour;
  vec3 new_vel = cur_vel + inj_vel;

  out_colour = vec4(new_vel, 1.0f);
}