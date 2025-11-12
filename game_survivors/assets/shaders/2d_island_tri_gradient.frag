// version prepended to file when loaded by engine.
//

out vec4 out_colour;

in VS_OUT
{
  vec2 v_uv;
  vec4 v_colour;
  vec2 v_sprite_pos;  // x, y location of sprite
  vec2 v_sprite_size; // e.g. 16, 16
  vec2 v_sprite_wh;   // desired sprites e.g. 2, 2
  vec2 v_sprite_max;  // 22 sprites
  vec3 v_sprite_global_pos_and_rot;
  float v_tex_unit;
} fs_in;

layout(std140) uniform Data {
  mat4 projection_zoomed;
	mat4 view;
	vec4[32] light_positions;
  vec2 camera_pos;
  vec2 screenshake;
  float time;
  float zoom;
  float tilesize;
};

uniform sampler2D tex_island_triangles;
uniform vec2 screen_wh;

void
main()
{
  vec2 v_uv = fs_in.v_uv;
  vec4 v_colour = fs_in.v_colour;

  vec3 tex_col = texture(tex_island_triangles, v_uv).rgb;

	vec2 cam_uv = camera_pos + (v_uv * screen_wh);
  cam_uv /= screen_wh;

  vec2 p = vec2(1.0);
  int levels = 11;

  float n = 0.5f;
  vec3 col = tex_col * vec3(n, n, n) * vec3(0.15, 0.8, 0.7);

  out_colour.rgb = col;
  out_colour.a = 1.0f;
}