// version prepended to file when loaded by engine.
//



layout(location = 0) in vec4 vertex; // xy and uv
// layout(location = 1) in vec4 pos_and_size;
layout(location = 1) in vec4 colour;
layout(location = 2) in vec4 sprite_pos;
layout(location = 3) in vec4 sprite_width_and_max;
layout(location = 4) in float tex_unit;
layout(location = 5) in vec4 parallax; // xy: translational offset. wz: rotation
layout(location = 6) in mat4 model;

out VS_OUT 
{
  vec2 v_uv;
  vec4 v_colour;
  vec2 v_sprite_pos;
  vec2 v_sprite_wh;
  vec2 v_sprite_max;
  float v_tex_unit;
  vec2 v_vertex;
} vs_out;

// out vec2 v_pos;
// out vec2 v_size;

// mat4 projection;
layout(std140) uniform Data {
  mat4 projection_zoomed;
  mat4 view;
  vec2 camera_pos;
  float time;
  float zoom;
  float tilesize;
};

uniform mat4 projection;
uniform bool is_fullscreen;
uniform bool do_zoom;

mat4 parallax_rotation_matrix(vec2 rotation) {

  mat4 x_rot_mat = mat4(
    1.0, 0.0, 0.0, 0.0,
    0.0, cos(rotation.x), -sin(rotation.x), 0.0,
    0.0, sin(rotation.x), cos(rotation.x), 0.0,
    0.0, 0.0, 0.0, 1.0
  );

  mat4 y_rot_mat = mat4(
    cos(rotation.y), 0.0, sin(rotation.y), 0.0,
    0.0, 1.0, 0.0, 0.0,
    -sin(rotation.y), 0.0, cos(rotation.y), 0.0,
    0.0, 0.0, 0.0, 1.0
  );

  return y_rot_mat * x_rot_mat;
}

mat4 parallax_offset_matrix(vec2 offset){
  mat4 offset_mat = mat4(
      1.0, 0.0, 0.0, 0.0,
      0.0, 1.0, 0.0, 0.0,
      0.0, 0.0, 1.0, 0.0,
      offset.x, offset.y, 0.0, 1.0
  );
  return offset_mat;
}

void
main()
{
  vs_out.v_uv = vertex.zw;
  vs_out.v_colour = colour;
  vs_out.v_sprite_pos = sprite_pos.xy;
  vs_out.v_sprite_wh = sprite_width_and_max.xy;
  vs_out.v_sprite_max = sprite_width_and_max.zw;
  vs_out.v_tex_unit = tex_unit;
  vs_out.v_vertex = vec4( model * vec4(vertex.xy, 1.0, 1.0)).xy;

  mat4 final_view = is_fullscreen ? mat4(1.0) : view;
  mat4 final_proj = do_zoom ? projection_zoomed : projection;

  // mat4 to_center = mat4(
  //   1.0, 0.0, 0.0, 0.0,
  //   0.0, 1.0, 0.0, 0.0,
  //   0.0, 0.0, 1.0, 0.0,
  //   -0.5, -0.5, 0.0, 1.0
  // );

  // mat4 from_center = mat4(
  //   1.0, 0.0, 0.0, 0.0,
  //   0.0, 1.0, 0.0, 0.0,
  //   0.0, 0.0, 1.0, 0.0,
  //   0.5, 0.5, 0.0, 1.0
  // );

  // mat4 parallax_offset = parallax_offset_matrix(parallax.xy);
  // mat4 parallax_rotation = parallax_rotation_matrix(parallax.wz);
  // mat4 parallax_mat = parallax_offset * from_center * parallax_rotation * to_center;

  // gl_Position = projection * view * model * parallax_mat * vec4(vertex.xy, 0.0, 1.0);
  gl_Position = final_proj * final_view * model * vec4(vertex.xy, 0.0, 1.0);

  // if(shake)
  // {
  //   // todo: translational and rotational screenshake
  //   gl_Position.x += cos(time * 10.0f) * strength;        
  //   gl_Position.y += cos(time * 15.0f) * strength;    
  //   gl_Position.z *= cos(time * 100.0f) * strength; 
  // }
}