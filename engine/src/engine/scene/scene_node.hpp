#pragma once

// c++ standard library
#include <memory>
#include <vector>

// other library headers
#include <GL/glew.h>

// your project headers
#include "engine/graphics/material.hpp"
#include "engine/mesh/mesh.hpp"

namespace fightingengine {

class SceneNode
{
public:
  // each node contains relevant render state
  std::shared_ptr<Mesh> Mesh;
  std::shared_ptr<Material> Material;

  // bounding box
  glm::vec3 BoxMin = glm::vec3(-99999.0f);
  glm::vec3 BoxMax = glm::vec3(99999.0f);

private:
  // per-node transform (w/ parent-child relationship)
  glm::mat4 transform_;
  glm::mat4 prev_transform_;

  glm::vec3 position_ = glm::vec3(0.0f);
  glm::vec4 rotation_; // axis-angle for now
  glm::vec3 scale_ = glm::vec3(1.0f);

  // mark the current node's tranform as dirty
  // if it needs to be re-calculated this frame
  bool dirty_;

  // each node is uniquely identified by
  // a 32-bit incrementing unsigned integer
  unsigned int id_;

public:
  SceneNode(unsigned int id);
  ~SceneNode();

  // scene node transform
  void set_position(glm::vec3 position);
  void set_rotation(glm::vec4 rotation);
  void set_scale(glm::vec3 scale);
  void set_scale(float scale);
  [[nodiscard]] glm::vec3 get_local_position() const;
  [[nodiscard]] glm::vec4 get_local_rotation() const;
  [[nodiscard]] glm::vec3 get_local_scale() const;
  [[nodiscard]] glm::vec3 get_world_position();
  [[nodiscard]] glm::vec3 get_world_scale();

  // returns the transform of the current node
  // combined with its parent(s)' transform.
  [[nodiscard]] glm::mat4 get_transform();
  [[nodiscard]] glm::mat4 get_prev_transform() const;

  // re-calculates this node and its children's
  // transform components if its parent or the
  // node itself is dirty.
  void update_transform(bool updatePrevTransform = false);
};
}
