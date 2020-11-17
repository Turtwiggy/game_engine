#version 330 core

// vec2 position, vec2 tex
layout (location = 0) in vec3 pos; 
layout (location = 1) in vec2 uv; 

out vec2 TexCoords;

uniform mat4 model;
uniform mat4 projection;

void main()
{
    TexCoords = uv.xy;
    gl_Position = projection * model * vec4(pos.xy, 0.0, 1.0);
}