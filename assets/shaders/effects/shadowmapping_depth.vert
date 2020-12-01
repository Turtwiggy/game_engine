#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec2 aTexCoords;

out VS_OUT {
    vec3 FragPos;
    vec3 Normal;
    vec2 TexCoords;
    vec4 FragPosLightSpace;
} vs_out;

uniform mat4 view_projection;
uniform mat4 light_space_matrix;
uniform mat4 model;

void main()
{
    //light-space transformation
    vs_out.FragPos = vec3(model * vec4(aPos, 1.0));
    vs_out.Normal = transpose(inverse(mat3(model))) * aNormal;
    vs_out.TexCoords = aTexCoords;
    vs_out.FragPosLightSpace = light_space_matrix * vec4(vs_out.FragPos, 1.0);
    
    gl_Position = view_projection * vec4(vs_out.FragPos, 1.0);
}  