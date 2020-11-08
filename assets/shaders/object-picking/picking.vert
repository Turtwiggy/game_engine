#version 330
layout (location = 0) in vec3 aPosition;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec2 aTexCoords;

uniform mat4 pvm;

void main()
{
    // only need to transform the vertex position.
    gl_Position = pvm * vec4(aPosition, 1.0);
}