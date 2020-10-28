#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec3 aTex;

out vec3 TexCoords;

uniform mat4 view_projection;

void main()
{
    TexCoords = aPos;
    vec4 pos = view_projection * vec4(aPos, 1.0);
    gl_Position = pos.xyww;
}  