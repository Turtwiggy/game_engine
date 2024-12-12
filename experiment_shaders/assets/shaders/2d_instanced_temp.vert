// version prepended to file when loaded by engine.
//

layout(location = 0) in vec4 vertex; // xy and uv
layout(location = 1) in vec4 colour;
layout(location = 2) in vec4 sprite_pos;
layout(location = 3) in vec4 sprite_width_and_max;
layout(location = 4) in float tex_unit;
layout(location = 5) in vec2 center;
layout(location = 6) in mat4 model;

out vec2 v_uv;
out vec4 v_colour;
out vec2 v_sprite_pos;
out vec2 v_sprite_wh;
out vec2 v_sprite_max;
out float v_tex_unit;
// out vec2 v_vertex;

uniform mat4 view;
uniform mat4 projection;
uniform bool shake;
uniform float time;
uniform float strength;
uniform vec2 mouse_pos;

// Create a 3D rotation matrix for X and Y rotations
mat4 parallaxRotationMatrix(float x_rot, float y_rot) {
  mat4 x_rot_mat = mat4(
    1.0, 0.0, 0.0, 0.0,
    0.0, cos(x_rot), -sin(x_rot), 0.0,
    0.0, sin(x_rot), cos(x_rot), 0.0,
    0.0, 0.0, 0.0, 1.0
  );
  mat4 y_rot_mat = mat4(
    cos(y_rot), 0.0, sin(y_rot), 0.0,
    0.0, 1.0, 0.0, 0.0,
    -sin(y_rot), 0.0, cos(y_rot), 0.0,
    0.0, 0.0, 0.0, 1.0
  );
  return y_rot_mat * x_rot_mat;
}

mat4 parallaxOffsetMatrix(vec2 offset){
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
  v_uv = vertex.zw;
  v_colour = colour;
  v_sprite_pos = sprite_pos.xy;
  v_sprite_wh = sprite_width_and_max.xy;
  v_sprite_max = sprite_width_and_max.zw;
  v_tex_unit = tex_unit;
  vec2 v_vertex = vec4( model * vec4(vertex.xy, 0.0, 1.0)).xy;

  mat4 to_center = mat4(
    1.0, 0.0, 0.0, 0.0,
    0.0, 1.0, 0.0, 0.0,
    0.0, 0.0, 1.0, 0.0,
    -0.5, -0.5, 0.0, 1.0
  );

  mat4 from_center = mat4(
    1.0, 0.0, 0.0, 0.0,
    0.0, 1.0, 0.0, 0.0,
    0.0, 0.0, 1.0, 0.0,
    0.5, 0.5, 0.0, 1.0
  );

  mat4 parallax_mat = mat4(1.0f);
  vec2 diff = (mouse_pos - center);
  float distance_from_center = length(diff);
  if(distance_from_center < 50) {
    vec2 diff_adj = diff / 50.0f;
    float x_rot = clamp(-diff_adj.y, -0.5, 0.5); // Pitch
    float y_rot = clamp(diff_adj.x , -0.5, 0.5) ;  // Yaw

    vec2 offset = diff * (distance_from_center * 0.00000);

    mat4 parallax_offset = parallaxOffsetMatrix(offset);
    mat4 parallax_rotation = parallaxRotationMatrix(x_rot, y_rot);
    
    parallax_mat = parallax_offset * from_center * parallax_rotation * to_center;
  }
  
  gl_Position = projection * view * model * parallax_mat * vec4(vertex.xy, 0.0, 1.0);

  if(shake)
  {
    // todo: translational and rotational screenshake
    gl_Position.x += cos(time * 10.0f) * strength;        
    gl_Position.y += cos(time * 15.0f) * strength;    
    // gl_Position.z *= cos(time * 100.0f) * strength; 
  }
}