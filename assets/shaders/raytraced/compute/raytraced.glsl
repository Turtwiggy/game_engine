#version 430 core

layout(binding = 0, rgba32f) uniform image2D positionData;
layout(binding = 1, rgba16f) uniform image2D normalData;
layout(binding = 2, rgba16f) uniform image2D albedoSpecData;

uniform vec3 eye, ray00, ray01, ray10, ray11;

layout (local_size_x = 16, local_size_y = 8) in;

void main(void) {

  ivec2 px = ivec2(gl_GlobalInvocationID.xy);

  ivec2 size = imageSize(albedoSpecData);

  //vec3 color = trace(eye, normalize(dir));
  //vec3 color = vec3(0.0, 1.0, 0.0);
  vec3 color = imageLoad(albedoSpecData, px).rgb;

  color.x = 1.0 - color.x;
  color.y = 1.0 - color.y;
  color.z = 1.0 - color.z;

  imageStore(albedoSpecData, px, vec4(color, 1.0));
}
