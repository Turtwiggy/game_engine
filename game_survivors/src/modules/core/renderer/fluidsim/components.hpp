#pragma once

/*

https://developer.nvidia.com/gpugems/gpugems/part-vi-beyond-triangles/chapter-38-fast-fluid-dynamics-simulation-gpu
https://github.com/PavelDoGreat/WebGL-Fluid-Simulation/blob/master/script.js#L856
https://code.tutsplus.com/how-to-write-a-smoke-shader--cms-25587t
https://github.com/mharrys/fluids-2d/blob/master/shaders/advect.fs
https://github.com/Turtwiggy/game_engine/commit/4addc1b7902598f0468c4f36ad81c74915e35466

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
  std::array<FboAndTexInfo, 2> info;

  FboAndTexInfo read() const { return info[0]; };
  FboAndTexInfo write() const { return info[1]; };
  void swap()
  {
    FboAndTexInfo temp = info[0];
    info[0] = info[1];
    info[1] = temp;
  };
};

struct FluidSimData
{
  float config_sim_resolution = 512;
  float config_dye_resolution = 2048;
  float config_density_dissapation = 1.0f;
  float config_velocity_dissapation = 0.2f;
  float config_pressure = 0.8f;
  float config_pressure_iterations = 20;
  float config_splat_radius = 0.005f;
  float config_splat_force = 100.0f;
  float config_curl = 30.0f;
  glm::vec3 config_dye_colour = { 1.0f, 1.0f, 1.0f };

  engine::Shader splatProgram;
  engine::Shader advectProgram;
  engine::Shader curlProgram;
  engine::Shader vorticityProgram;
  engine::Shader divergenceProgram;
  engine::Shader pressureProgram;
  engine::Shader gradientSubtractProgram;
  engine::Shader textureProgram;

  DoubleBufferInfo dye;
  DoubleBufferInfo velocity;
  SingleBufferInfo divergence;
  SingleBufferInfo curl;
  DoubleBufferInfo pressure;
};

} // namespace game2d