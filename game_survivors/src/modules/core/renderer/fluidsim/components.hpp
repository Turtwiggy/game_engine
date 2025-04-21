#pragma once

/*

https://developer.nvidia.com/gpugems/gpugems/part-vi-beyond-triangles/chapter-38-fast-fluid-dynamics-simulation-gpu
https://github.com/PavelDoGreat/WebGL-Fluid-Simulation/blob/master/script.js#L856
https://code.tutsplus.com/how-to-write-a-smoke-shader--cms-25587t
https://github.com/mharrys/fluids-2d/blob/master/shaders/advect.fs

GPU Gems steps are:
- Advect
- Diffuse
- Add Forces
- Pressure
- Subtract pressure Gradient.

Some implementations:
- Calculate divergence
- for some number of iterations (between 40-80, minimum 20)
  - Calculate pressure (via jacobi)
- Subtract pressure gradient.
- Advect (self)
- Advect (dye)

Additionally, a full solution might look like:
- Calculate forces (mouse, gravity)
- Calculate curl, vorticity
- Calculate divergence
- Calculate pressure (via jacobi)
- Subtract pressure gradient.
- Advect (self)
- Advect (dye)
*/

#include "engine/opengl/framebuffer.hpp"
#include "engine/opengl/shader.hpp"
#include "modules/core/renderer/helpers/texture.hpp"

#include <array>

namespace game2d {

struct FboAndTexInfo
{
  engine::FramebufferID fbo_id;
  Texture tex;
};

struct SingleBufferInfo
{
  FboAndTexInfo info;
};

struct DoubleBufferInfo
{
  int index = 0;
  std::array<FboAndTexInfo, 2> info;
  void swap() { index = index == 0 ? 1 : 0; }
};

struct FluidSimData
{
  float config_sim_resolution = 1024;
  float config_dye_resolution = 1024;
  float config_density_dissapation = 1.0f;
  float config_velocity_dissapation = 0.2f;
  float config_pressure = 0.8f;
  float config_pressure_iterations = 20;
  float config_splat_radius = 0.25f;
  float config_splat_force = 6000.0f;

  engine::Shader splatProgram;
  engine::Shader advectProgram;
  engine::Shader divergenceProgram;
  engine::Shader pressureProgram;
  engine::Shader gradientSubtractProgram;

  DoubleBufferInfo dye;
  DoubleBufferInfo velocity;
  SingleBufferInfo divergence;
  DoubleBufferInfo pressure;
};

} // namespace game2d