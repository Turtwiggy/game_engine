// version prepended to file when loaded by engine.
//

out vec4 out_colour;

in SIM_VS_OUT
{
  vec2 v_uv;
} sim_fs_in;

uniform float dt;
uniform vec2 texel_size;
uniform sampler2D u_velocity;
uniform sampler2D u_source;
uniform float dissapation;

vec4 bilerp (sampler2D sam, vec2 uv, vec2 tsize) {
    vec2 st = uv / tsize - 0.5;

    vec2 iuv = floor(st);
    vec2 fuv = fract(st);

    vec4 a = texture(sam, (iuv + vec2(0.5, 0.5)) * tsize);
    vec4 b = texture(sam, (iuv + vec2(1.5, 0.5)) * tsize);
    vec4 c = texture(sam, (iuv + vec2(0.5, 1.5)) * tsize);
    vec4 d = texture(sam, (iuv + vec2(1.5, 1.5)) * tsize);

    return mix(mix(a, b, fuv.x), mix(c, d, fuv.x), fuv.y);
}

void
main()
{
  vec2 v_uv = sim_fs_in.v_uv;

  // vec2 coord = v_uv - dt * bilerp(u_velocity, v_uv, texel_size).xy * texel_size;
  // vec4 result = bilerp(u_source, coord, dye_texel_size);

  vec2 cur_vel = texture(u_velocity, v_uv).xy;
  vec2 coord = v_uv - dt * cur_vel * texel_size ; // back in time.
  vec4 result = texture(u_source, coord);

  out_colour = result;

  // decay
  out_colour.rgb /= (1.0 + dt * dissapation);

}