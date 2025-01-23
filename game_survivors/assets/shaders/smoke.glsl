// version prepended to file via engine
//

// https://learnopengl.com/Guest-Articles/2022/Compute-Shaders/Introduction
// https://code.tutsplus.com/how-to-write-a-smoke-shader--cms-25587t
// https://github.com/mharrys/fluids-2d/blob/master/shaders/advect.fs
// https://github.com/PavelDoGreat/WebGL-Fluid-Simulation/blob/master/script.js#L856
// https://www.shadertoy.com/view/4tGfDW

// limitation of work groups
// that can be dispatched iin a single compute shader dispatch call
// check: GL_MAX_COMPUTE_WORK_GROUP_COUNT

// limitation on local size
// check: GL_MAX_COMPUTE_WORK_GROUP_SIZE

// limitation on the total number of invocations in a work group,
// which is that the product of the X, Y, Z components of the
// local size must be less than
// check: GL_MAX_COMPUTE_WORK_GROUP_INVOCATIONS

// The total amount of n dimensional executions is
// the product of the amount of work groups times local invocations.
// For example, for a texture wh of (1000, 1000)
// if you glDispatchCompute(100, 100, 1), you'd execute 10'000 work groups.
// (1000*1000) = 1'000'000/100 = 10'000 work groups.
// if local_size was (1, 1, 1), you'd only see (1/100th of the image)
// adjust local_size to (10, 10, 1) to see the full image (10x10)=100

layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;

layout(rgba32f, binding = 0) uniform image2D img_output;
layout(rgba32f, binding = 1) uniform image2D fluiddata;     // x: vel_x, y: vel_y, z: pressure, a: divergence
layout(rgba32f, binding = 2) uniform image2D vorticitydata; // (X) curl
layout(rgba32f, binding = 3) uniform image2D u_dye;

// layout(location = 0) uniform float t;
layout(location = 0) uniform vec2 viewport_wh;
layout(location = 1) uniform vec2 mouse_pos;
layout(location = 2) uniform vec2 mouse_delta;

const float dt = 1/60.0;
const ivec2 gridsize = ivec2(1280, 720); 
const float DIFFUSION_ITERATIONS = 2; // usualy 20-50
const float PRESSURE_JACOBI_ITERATIONS = 6; // 40 to 80

// const float pressure = 0.8; // clear the screen to a pressure?
const float velocity_dissipation = 0.2;     // self-advection
const float density_dissipation = 0.9;      // dye

const float viscosity = 0.1;        // affects diffusion
const float curl_amount = 5.0;
const float brush_pressure = 10.0 ;
const float brush_radius = 100.0;

bool 
oob(ivec2 pos)
{
  return pos.x < 0 || pos.y < 0 || pos.x >= int(gridsize.x) || pos.y >= int(gridsize.y);
}

vec2 sample_velocity(ivec2 pos) {
  if (oob(pos)) 
    return vec2(0.0);
  return imageLoad(fluiddata, pos).xy;
}

float sample_pressure(ivec2 pos) {
  if (oob(pos)) 
    return 0.0;     
  return imageLoad(fluiddata, pos).z;
}

float sample_divergence(ivec2 pos) {
  if (oob(pos)) 
    return 0.0;
  return imageLoad(fluiddata, pos).a;
}

void
advect_vel()
{
  ivec2 pixel = ivec2(gl_GlobalInvocationID.xy);
  vec2 cur_vel = sample_velocity(pixel);
  ivec2 coord = pixel - ivec2(dt * cur_vel); 

  if(oob(coord)) 
    return;

  vec2 result = imageLoad(fluiddata, coord).xy;
  float decay = 1.0 + velocity_dissipation * dt;
  vec2 advected = result / decay;

  imageStore(fluiddata, pixel, vec4(advected, imageLoad(fluiddata, pixel).ba));
}

void
advect_dye()
{
  ivec2 pixel = ivec2(gl_GlobalInvocationID.xy);
  vec2 cur_vel = sample_velocity(pixel);
  ivec2 coord = pixel - ivec2(dt * cur_vel); 

  if(oob(coord)) 
    return;

  vec3 result = imageLoad(u_dye, coord).xyz;
  float decay = 1.0 + density_dissipation * dt;
  vec3 advected = result / decay;
  imageStore(u_dye, pixel, vec4(advected, 1.0));
}

void
diffuse()
{
  ivec2 pixel = ivec2(gl_GlobalInvocationID.xy);

  // Boundary-safe sampling
  ivec2 sL = clamp(pixel - ivec2(1, 0), ivec2(0), ivec2(gridsize) - 1);
  ivec2 sR = clamp(pixel + ivec2(1, 0), ivec2(0), ivec2(gridsize) - 1);
  ivec2 sB = clamp(pixel - ivec2(0, 1), ivec2(0), ivec2(gridsize) - 1);
  ivec2 sT = clamp(pixel + ivec2(0, 1), ivec2(0), ivec2(gridsize) - 1);

  vec2 sum = vec2(0.0);
  sum += sample_velocity(sL);
  sum += sample_velocity(sR);
  sum += sample_velocity(sB);
  sum += sample_velocity(sT);

  vec2 cur_vel = imageLoad(fluiddata, pixel).xy;

  // float alpha = (dt * viscosity);
  // float beta = 1.0 / (1.0 + 4.0 * alpha);
  // vec2 new_vel = (sum + alpha * cur_vel) * beta;

  // vec2 new_vel = (sum - vec2(4.0) * cur_vel);
  vec2 new_vel = cur_vel + viscosity * (sum - 4.0 * cur_vel);

  // update
  imageStore(fluiddata, pixel, vec4(new_vel, imageLoad(fluiddata, pixel).ba));
}

float gauss(vec2 p, float r)
{
  return exp(-dot(p, p) / r);
}

void
apply_forces()
{
  ivec2 pixel = ivec2(gl_GlobalInvocationID.xy);

  vec2 p = vec2(pixel) - vec2(mouse_pos);
  float dist = distance(vec2(pixel), vec2(mouse_pos));

  if(dist >= brush_radius)
    return; // todo: remove this if statement

  if(length(mouse_delta) <= 0.0)
    return; // todo: remove this if statement

  // float falloff = 1.0 - smoothstep(0.0, brush_radius, dist);
  float splat = gauss(p, brush_radius);

  // Update velocity with splat
  vec4 data = imageLoad(fluiddata, pixel);
  vec2 inj_vel = mouse_delta * brush_pressure * splat;
  vec2 new_vel = data.xy + inj_vel;
  imageStore(fluiddata, pixel, vec4(new_vel, data.z, data.a ));

  // Update dye with splat
  vec4 dye_data = imageLoad(u_dye, pixel);
  vec3 col = vec3(1.0, 0.0, 0.0);
  vec3 col_final = dye_data.rgb + splat * col;
  imageStore(u_dye, pixel, vec4(col_final, 1.0));
}

void
curl()
{
  ivec2 pixel = ivec2(gl_GlobalInvocationID.xy);

  // Boundary-safe sampling
  ivec2 sL = clamp(pixel - ivec2(1, 0), ivec2(0), ivec2(gridsize) - 1);
  ivec2 sR = clamp(pixel + ivec2(1, 0), ivec2(0), ivec2(gridsize) - 1);
  ivec2 sB = clamp(pixel - ivec2(0, 1), ivec2(0), ivec2(gridsize) - 1);
  ivec2 sT = clamp(pixel + ivec2(0, 1), ivec2(0), ivec2(gridsize) - 1);

  // Get velocities
  float vL = imageLoad(fluiddata, sL).y;
  float vR = imageLoad(fluiddata, sR).y;
  float vB = imageLoad(fluiddata, sB).x;
  float vT = imageLoad(fluiddata, sT).x;

  float vorticity = 0.5 * (vR - vL - vT + vB);

  imageStore(vorticitydata, pixel, vec4(vorticity, imageLoad(vorticitydata, pixel).gba ));
}

void
vorticity()
{
  ivec2 pixel = ivec2(gl_GlobalInvocationID.xy);

  // Boundary-safe sampling
  ivec2 sL = clamp(pixel - ivec2(1, 0), ivec2(0), ivec2(gridsize) - 1);
  ivec2 sR = clamp(pixel + ivec2(1, 0), ivec2(0), ivec2(gridsize) - 1);
  ivec2 sB = clamp(pixel - ivec2(0, 1), ivec2(0), ivec2(gridsize) - 1);
  ivec2 sT = clamp(pixel + ivec2(0, 1), ivec2(0), ivec2(gridsize) - 1);

  float vL = abs(imageLoad(vorticitydata, sL).x);
  float vR = abs(imageLoad(vorticitydata, sR).x);
  float vB = abs(imageLoad(vorticitydata, sB).x);
  float vT = abs(imageLoad(vorticitydata, sT).x);
  float vC = imageLoad(vorticitydata, pixel).x;

  vec2 force = 0.5 * vec2(vT - vB, vR - vL);
  force /= length(force) + 0.0001;
  force *= curl_amount * vC;
  force.y *= -1.0f;

  vec2 vel = imageLoad(fluiddata, pixel).xy;
  vel += force * dt;
  vel = min(max(vel, -1000.0), 1000.0f);

  // Store updated velocity
  imageStore(fluiddata, pixel, vec4(vel, imageLoad(fluiddata, pixel).ba));
}

void
divergence()
{
  ivec2 pixel = ivec2(gl_GlobalInvocationID.xy);

  // Boundary-safe sampling
  ivec2 sL = clamp(pixel - ivec2(1, 0), ivec2(0), ivec2(gridsize) - 1);
  ivec2 sR = clamp(pixel + ivec2(1, 0), ivec2(0), ivec2(gridsize) - 1);
  ivec2 sB = clamp(pixel - ivec2(0, 1), ivec2(0), ivec2(gridsize) - 1);
  ivec2 sT = clamp(pixel + ivec2(0, 1), ivec2(0), ivec2(gridsize) - 1);

  float wL = imageLoad(fluiddata, sL).x;
  float wR = imageLoad(fluiddata, sR).x;
  float wB = imageLoad(fluiddata, sB).y;
  float wT = imageLoad(fluiddata, sT).y;

  vec2 C = sample_velocity(pixel);
  if (oob(sL)) { wL = -C.x; }
  if (oob(sR)) { wR = -C.x; }
  if (oob(sT)) { wT = -C.y; }
  if (oob(sB)) { wB = -C.y; }
  
  // float rdx = 1.0 / gridscale;
  float div = 0.5 * ( (wR - wL) + (wT - wB) );

  // Update divergence in alpha channel
  imageStore(fluiddata, pixel, vec4(imageLoad(fluiddata, pixel).rgb, div));
}

void
solve_pressure_jacobi()
{
  ivec2 pixel = ivec2(gl_GlobalInvocationID.xy);

  // Boundary-safe sampling
  ivec2 sL = clamp(pixel - ivec2(1, 0), ivec2(0), ivec2(gridsize) - 1);
  ivec2 sR = clamp(pixel + ivec2(1, 0), ivec2(0), ivec2(gridsize) - 1);
  ivec2 sB = clamp(pixel - ivec2(0, 1), ivec2(0), ivec2(gridsize) - 1);
  ivec2 sT = clamp(pixel + ivec2(0, 1), ivec2(0), ivec2(gridsize) - 1);

  // sample pressure
  float xL = imageLoad(fluiddata, sL).z;
  float xR = imageLoad(fluiddata, sR).z;
  float xB = imageLoad(fluiddata, sB).z;
  float xT = imageLoad(fluiddata, sT).z;
  
  float div = sample_divergence(pixel);
  float alpha = 1.0; 
  float rBeta = 0.25; // Reciprocal of 4
  // float new_p = (xL + xR + xB + xT + alpha * bC) * rBeta;
  float new_p = (xL + xR + xB + xT - div) * 0.25;

  // TRIAL: pressure relaxation?
  // float pressure_relax = 0.1;
  // new_p *= pressure_relax;

  // Update pressure in z (or blue) channel
  vec4 data = imageLoad(fluiddata, pixel);
  imageStore(fluiddata, pixel, vec4(data.xy, new_p, data.w));
}

void
subtract_pressure_gradient()
{
  ivec2 pixel = ivec2(gl_GlobalInvocationID.xy);

  // Boundary-safe sampling
  ivec2 sL = clamp(pixel - ivec2(1, 0), ivec2(0), ivec2(gridsize) - 1);
  ivec2 sR = clamp(pixel + ivec2(1, 0), ivec2(0), ivec2(gridsize) - 1);
  ivec2 sB = clamp(pixel - ivec2(0, 1), ivec2(0), ivec2(gridsize) - 1);
  ivec2 sT = clamp(pixel + ivec2(0, 1), ivec2(0), ivec2(gridsize) - 1);

  // Pressure gradients
  // float p = imageLoad(fluiddata, pixel).z;
  float pL = sample_pressure(sL);
  float pR = sample_pressure(sR);
  float pB = sample_pressure(sB);
  float pT = sample_pressure(sT);
  
  vec2 cur_vel = sample_velocity(pixel);

  // WARNING: might be missing 0.5 mul here
  // vec2 new_vel = cur_vel - vec2(pR - pL, pT - pB);

  // float halfrdx = 0.5 / gridscale;
  vec2 new_vel = cur_vel - vec2(
    0.5 * (pR - pL),
    0.5 * (pT - pB)
  );

  // store new velocity in xy...
  imageStore(fluiddata, pixel, vec4(new_vel, imageLoad(fluiddata, pixel).za));
}

void
main()
{
  ivec2 pixel = ivec2(gl_GlobalInvocationID.xy);
  vec4 value = vec4(0.0, 0.0, 0.0, 1.0);

  /*
  // float v_uv_x = float(pixel.x) / (gl_NumWorkGroups.x * gl_WorkGroupSize.x);
  // float v_uv_y = float(pixel.y) / (gl_NumWorkGroups.y * gl_WorkGroupSize.y);
  // value.r = v_uv_x;
  // value.g = v_uv_y;

  int x_pixel = 1;
  int y_pixel = 1;

  // colour of the current pixel
  vec4 col = imageLoad(img_output, pixel);
  vec4 right = imageLoad(img_output, pixel + ivec2(x_pixel, 0));
  vec4 left = imageLoad(img_output, pixel - ivec2(x_pixel, 0));
  vec4 up = imageLoad(img_output, pixel + ivec2(0, y_pixel));
  vec4 down = imageLoad(img_output, pixel - ivec2(0, y_pixel));

  // output same as last frame
  value.rgb = col.rgb; 

  // generate smoke when mouse is pressed
  float dist = distance(mouse_pos, pixel);
  float smoke_source = 0.01f; // could be some passed in float for pressure
  float brush_radius = 50.0f;
  value.rgb += smoke_source * max(brush_radius - dist, 0.0);

  // Diffuse equation
  // note: it's easy to make the simulation "blow up"
  // if you gain more than you lose, things cant diffuse ("bad diffuse").
  float dissipation = 14.0 * 0.016; // lower val = more dissipation
  float factor = dissipation * (0.25 * (left.r + right.r + down.r + up.r) - col.r );

  // Account for the low precision of texels
  float minimum = 0.003;
  if (factor >= -minimum && factor < 0.0)
    factor = -minimum;
  value.rgb += vec3(factor);

  value.r = clamp(value.r, 0, 1);
  value.g = clamp(value.g, 0, 1);
  value.b = clamp(value.b, 0, 1);
  imageStore(img_output, pixel, value);
  */

  // GpuGems pseudo code...
  // u = advect(u);
  // u = diffuse(u);
  // u = add_forces(u);
  // p = compute_pressure(u);
  // u = subtract_pressure_gradient(u, p);

  // Advect velocity (self-advection)
  advect_vel();
  barrier();

  // Advect density (dye)
  advect_dye();
  barrier();

  for(int i = 0; i < DIFFUSION_ITERATIONS; i++){
    diffuse();
    barrier();
  }

  apply_forces();
  barrier();

  curl();
  barrier();

  vorticity();
  barrier();

  divergence();
  barrier();

  for(int i = 0; i < PRESSURE_JACOBI_ITERATIONS; i++){
    solve_pressure_jacobi();
    barrier();
  }

  subtract_pressure_gradient();
  barrier();

  // 
  // what to display?
  //

  vec2 vel = sample_velocity(pixel);
  value.r = (vel.x + 1) / 2.0f;
  value.g = (vel.y + 1) / 2.0f;
  // value.r = length(vel.x + vel.y);
  value.b = sample_pressure(pixel);
  // value.g = imageLoad(fluiddata, pixel).a; // divergence
  // value.g = imageLoad(vorticitydata, pixel).x; // vorticity

  // value.rgb = imageLoad(u_dye, pixel).rgb;
  value.a = 1.0f;

  // clamp fluiddata values...?
  value.r = clamp(value.r, 0, 1);
  value.g = clamp(value.g, 0, 1);
  value.b = clamp(value.b, 0, 1);
  imageStore(img_output, pixel, value);
}