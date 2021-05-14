#version 330 core

layout (location = 0) in vec2 pos; 
layout (location = 2) in vec2 tex;
layout (location = 3) in mat4 instance_matrix;

out vec2 TexCoords;

// note that we're omitting the view matrix; the view never changes so we
// basically have an identity view matrix and can therefore omit it.
uniform mat4 projection;

void main()
{
    TexCoords = tex;
    gl_Position = projection * instance_matrix * vec4(pos, 0.0, 1.0);
}