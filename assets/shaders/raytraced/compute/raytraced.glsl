#version 430 core

layout(binding = 0, rgba16f) uniform image2D positionData;
layout(binding = 1, rgba16f) uniform image2D normalData;
layout(binding = 2, rgba16f) uniform image2D albedoSpecData;

uniform vec3 viewPos, ray00, ray01, ray10, ray11;

#define LARGE_FLOAT 1E+10
#define EPSILON 0.0001

layout (local_size_x = 16, local_size_y = 8) in;

vec3 trace(vec3 origin, vec3 dir, vec3 diffuse) {

        return diffuse;
}


void main(void) {

    ivec2 px = ivec2(gl_GlobalInvocationID.xy);
    ivec2 size = imageSize(albedoSpecData);

    if (any(greaterThanEqual(px, size)))
        return;

    /*
       * Now we take our rayNN uniforms declared above to determine the
       * world-space direction from the eye position through the current
       * work item's pixel's center in the framebuffer image. We use the
       * 'px' variable, cast it to a floating-point vector, offset it by
       * half a pixel's width (in whole pixel units) and then transform that
       * position relative to our framebuffer size to get values in the
       * interval [(0, 0), (1, 1)] for all work items covering our
       * framebuffer.
   */
    vec2 p = (vec2(px) + vec2(0.5)) / vec2(size);

    /*
       * Use bilinear interpolation based on the X and Y fraction
       * (within 0..1) with our rayNN vectors defining the world-space
       * vectors along the corner edges of the camera's view frustum. The
       * result is the world-space direction of the ray originating from the
       * camera/eye center through the work item's framebuffer pixel center.
    */
    vec3 dir = mix(mix(ray00, ray01, p.y), mix(ray10, ray11, p.y), p.x);

    //Sample textures
    vec3 FragPos = imageLoad(positionData, px).rgb;
    vec3 Normal = imageLoad(normalData, px).rgb;
    vec3 Diffuse = imageLoad(albedoSpecData, px).rgb;
    float Specular = imageLoad(albedoSpecData, px).a;

    vec3 color = trace(viewPos, normalize(dir), Diffuse);

    color.x = 1 - color.x;
    color.y = 1 - color.y;
    color.z = 1 - color.z;

    imageStore(albedoSpecData, px, vec4(color, 1.0));
}
