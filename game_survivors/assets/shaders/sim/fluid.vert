// version prepended to file when loaded by engine.
//

layout(location = 0) in vec4 vertex; // xy and uv
// layout(location = 1) in vec4 pos_and_size;
layout(location = 1) in vec4 colour;
layout(location = 2) in vec4 sprite_pos;
layout(location = 3) in vec4 sprite_width_and_max;
layout(location = 4) in float tex_unit;
layout(location = 5) in mat4 model;

out SIM_VS_OUT {
  vec2 v_uv;
} sim_vs_out;

void main() {
  sim_vs_out.v_uv = vertex.zw;
  // sim_vs_out.v_uv = vertex.xy * 0.5 + 0.5;

  // convert from [0, 1] to [-1, 1]
  vec2 adj_vertex =  2.0 * vertex.xy - vec2(1.0);

  gl_Position = vec4(adj_vertex, 0.0, 1.0);
}
