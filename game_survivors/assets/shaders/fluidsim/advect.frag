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

uniform sampler2D u_velocity;
uniform sampler2D u_source;
uniform vec2 texel_size;
uniform vec2 dye_texel_size;
uniform float dt;
uniform float dissapation;

// vec4 bilerp (sampler2D sam, vec2 uv, vec2 tsize) {
//   vec2 st = uv / tsize - 0.5;

//   vec2 iuv = floor(st);
//   vec2 fuv = fract(st);

//   vec4 a = texture2D(sam, (iuv + vec2(0.5, 0.5)) * tsize);
//   vec4 b = texture2D(sam, (iuv + vec2(1.5, 0.5)) * tsize);
//   vec4 c = texture2D(sam, (iuv + vec2(0.5, 1.5)) * tsize);
//   vec4 d = texture2D(sam, (iuv + vec2(1.5, 1.5)) * tsize);

//   return mix(mix(a, b, fuv.x), mix(c, d, fuv.x), fuv.y);
// }

void
main()
{
  vec2 v_uv = fs_in.v_uv;

  vec2 coord = v_uv - dt * texture2D(u_velocity, v_uv).xy * texel_size;
  vec4 result = texture2D(u_source, coord);

  float decay = 1.0 + dissapation * dt;

  out_colour = result / decay;
  // out_colour = vec4(1.0f, 0.0f, 0.0f, 1.0f);
}