// version prepended to file when loaded by engine.
//

layout(location = 0) in vec4 vertex; // xy and uv
layout(location = 1) in vec4 colour;
layout(location = 2) in mat4 model;

out VS_OUT {
  vec2 v_uv;
  vec4 v_colour;
} vs_out;

// out vec2 v_pos;
// out vec2 v_size;

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

uniform mat4 projection;
uniform bool is_fullscreen;
uniform bool do_zoom;

void main() {
  vs_out.v_uv = vertex.zw;
  vs_out.v_colour = colour;
  // vs_out.v_vertex = vec4(model * vec4(vertex.xy, 1.0, 1.0)).xy;

  mat4 final_view = is_fullscreen ? mat4(1.0) : view;
  mat4 final_proj = do_zoom ? projection_zoomed : projection;
 
  gl_Position = final_proj * final_view * model * vec4(vertex.xy, 0.0, 1.0);
}