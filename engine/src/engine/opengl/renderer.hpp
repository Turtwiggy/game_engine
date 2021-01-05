#pragma once

// c++ standard lib headers
#include <memory>
#include <vector>

// other library files
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/transform.hpp>

// your project headers
#include "engine/camera/camera.hpp"
#include "engine/camera/fly_camera.hpp"
#include "engine/core/application.hpp"
#include "engine/maths/random.hpp"
#include "engine/opengl/framebuffer.hpp"
#include "engine/opengl/passes/shadowmapping.hpp"
#include "engine/opengl/primitives.hpp"

namespace fightingengine {

class Renderer
{
public:
  Renderer(RandomState& rnd);

  void update(float delta_time,
              FlyCamera& camera,
              RandomState& rnd,
              const std::vector<glm::vec3>& cube_pos,
              const glm::ivec2& screen_size);

  // renderer info
  int draw_calls = 0;

  // renderpasses
  ShadowmappingRenderpass shadowmapping_pass;

private:
  // meshes available
  std::shared_ptr<Mesh> cube_;
  std::shared_ptr<Mesh> plane_;

  // skybox
  // Background* background_;
  // std::shared_ptr<TextureCube> cubemap_;

  // shaders available
  Shader shadowmap_shader_;

  // (temp) renderer state
  glm::vec3 light_pos_ = { 0.0f, 0.0f, 0.0f };

private:
  void render_scene(const std::vector<glm::vec3>& cube_pos, const glm::vec3& cam_pos, const Shader& shader);

  void draw_skybox(const glm::mat4& view_projection);
};

} // namespace fightingengine

// unsigned int sphereVAO = 0;
// unsigned int indexCount;