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

uniform sampler2D tex;
uniform vec2 viewport_wh;

layout(std140) uniform Data {
  mat4 projection_zoomed;
  mat4 view;
  vec2 camera_pos;
  float time;
  float zoom;
  float tilesize;
};

void
main()
{
  vec2 v_uv = fs_in.v_uv;
  vec4 v_colour= fs_in.v_colour;
  vec2 v_sprite_pos = fs_in.v_sprite_pos;
  vec2 v_sprite_wh = fs_in.v_sprite_wh;
  vec2 v_sprite_max = fs_in.v_sprite_max;
  int index = int(fs_in.v_tex_unit);

  // fragCoord : is a vec2 that is between 0 > 640 on the X axis and 0 > 360 on the Y axis,
  // where 640 is width and 360 is height.
  vec2 fragCoord = v_uv * viewport_wh * 2.0f;
  fragCoord.x *= -1.0f;
  fragCoord.y *= -1.0f;

  // based on: https://www.shadertoy.com/view/7lyyzd
  vec2 I = fragCoord + viewport_wh;
  vec2 fzoom = (I/4000.0f);
  vec2 scroll = vec2(time/1000.0f);
  vec2 pos = (camera_pos + vec2(1000.0f, -1000.0f)) / 1000.0f; // offset so never aligns

  vec4 O = vec4(0.0f, 0.0f, 0.0f, 1.0f);
  for(; O.r < texture(tex, fzoom*zoom - pos/O.r/100.0f - scroll).r; O+=0.02f);
  out_colour.rgb = O.rgb * 0.25f;

  // out_colour.rgb = vec3(0.0);
  out_colour.a = 1.0f;
}