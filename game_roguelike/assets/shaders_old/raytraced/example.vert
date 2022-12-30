#version 410 core
//layout (location = 0) in vec3 aPos;
//layout (location = 1) in vec2 aTexCoords;

/* The position of the vertex as two-dimensional vector */
in vec2 vertex;

out vec2 texcoord;

void main()
{
    //texcoord = aTexCoords;
    //gl_Position = vec4(aPos, 1.0);

    gl_Position = vec4(vertex, 0.0, 1.0);
    //Compute texture coordinate by simply interval-mapping from [-1..+1] to [0..1]
    texcoord = vertex * 0.5 + vec2(0.5, 0.5);
}
