#version 460 core

layout(location = 0) in vec3 v_pos;
layout(location = 1) in vec3 v_normal;
layout(location = 2) in vec2 v_uv;
layout(location = 3) in vec4 v_colour;
layout(location = 4) in ivec4 v_bone_ids;
layout(location = 5) in vec4 v_weights;

uniform mat4 projection;
uniform mat4 view;
uniform mat4 model;

const int MAX_BONES = 100;
const int MAX_BONE_INFLUENCE = 4;
uniform mat4 final_bone_matrices[MAX_BONES];

out vec2 o_uv;
out vec4 o_colour;

void
main()
{
  vec4 total_pos = vec4(0.0f);

  for(int i = 0; i < MAX_BONE_INFLUENCE; i++)
  {
    if(v_bone_ids[i] == -1)
      continue;
    if(v_bone_ids[i] >= MAX_BONES)
    {
      total_pos = vec4(v_pos, 1.0f);
      break;
    }
    vec4 local_pos = final_bone_matrices[v_bone_ids[i]] * vec4(v_pos, 1.0f);
    total_pos += local_pos * v_weights[i];
    // vec3 local_normal = mat3(final_bone_matrices[v_bone_ids[i]]) * v_normal;
  }

  // gl_Position = projection * view * model * vec4(v_pos, 1.0f);
  gl_Position = projection * view * model * total_pos;

  o_uv = v_uv;
  o_colour = v_colour;
}