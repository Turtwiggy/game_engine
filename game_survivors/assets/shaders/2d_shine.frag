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

layout(std140) uniform Data {
  mat4 projection_zoomed;
  mat4 view;
  vec2 camera_pos;
  vec2 screenshake;
  vec4[4] player_positions; // xy: pos, z: angle, w: active
  vec4[32] light_positions;
  float time;
  float zoom;
  float tilesize;
};

// this key is replaced by the engine with
// e.g. "uniform sampler2D tex_monochrome_transparent_packed"
{{ generate_user_samplers }}
uniform int RENDERER_TEX_UNIT_COUNT;

void
main()
{
  vec2 v_uv = fs_in.v_uv;
  vec4 v_colour = fs_in.v_colour;
  vec2 v_sprite_pos = fs_in.v_sprite_pos;
  vec2 v_sprite_wh = fs_in.v_sprite_wh;
  vec2 v_sprite_max = fs_in.v_sprite_max;
  int index = int(fs_in.v_tex_unit);

  out_colour.a = 1.0f;

  // the engine generates code here
  {
    // v_uv goes from 0 to 1
    // convert from 0 to 1 to the width/height desired
    vec2 sprite_uv = vec2(
      (v_sprite_wh.x * v_uv.x) / v_sprite_max.x + v_sprite_pos.x * (1.0f/v_sprite_max.x),
      (v_sprite_wh.y * v_uv.y) / v_sprite_max.y + v_sprite_pos.y * (1.0f/v_sprite_max.y)
    );
    vec4 col = vec4(1.0f);
{{ generate_sampler_if_statements }}
    out_colour = v_colour * col;
  }

  float u_time = time;
  vec3 shine_col = vec3(1.0, 0.3, 0.3);

  //
  // based off:
  // https://www.shadertoy.com/view/NtGczV
  //
  {
    vec3 a = mix(out_colour.rgb, vec3(1.), step(.995, (sin(v_uv.x-v_uv.y-u_time*2.0))));
    vec3 b = mix(a, vec3(1.), step(.997, (sin(v_uv.x-v_uv.y-(0.1+u_time)*2.0))));
    out_colour.rgb = shine_col * b;
  }
}