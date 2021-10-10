#pragma once

// engine headers
#include "engine/opengl/shader.hpp"

// other project headers
#include <entt/entt.hpp>
#include <glm/glm.hpp>

// c++ headers
#include <iostream>
#include <memory>
#include <vector>

namespace engine {

namespace triangle_renderer {

// data required per triangle
struct TriangleDescriptor
{
  glm::vec2 point_0 = { 0, 0 };
  glm::vec2 point_1 = { 0, 0 };
  glm::vec2 point_2 = { 0, 0 };
  glm::vec4 point_0_colour = glm::vec4(1.0f, 0.0f, 0.0f, 1.0f);
  glm::vec4 point_1_colour = glm::vec4(1.0f, 0.0f, 0.0f, 1.0f);
  glm::vec4 point_2_colour = glm::vec4(1.0f, 0.0f, 0.0f, 1.0f);
  int tex_slot = 0;
};

// data for vert.glsl
struct Vertex
{
  glm::vec4 pos_and_uv;
  glm::vec4 colour;
  glm::vec2 sprite_offset;
  float tex_slot;
  glm::mat4 model;
};

// interface for overriding how get triangle descriptors
struct ITriangleRenderable
{
  virtual std::vector<TriangleDescriptor>& get_triangles(entt::registry& registry) = 0;
};

class TriangleRenderer
{
public:
  inline static std::vector<std::unique_ptr<ITriangleRenderable>> interfaces;

public:
  // Note: I'm not sure how to move this template function
  // to the cpp file, as I was getting link errors.
  // have put as inline for the moment :/
  template<class T>
  inline static void register_interface()
  {
    std::cout << "(BatchTriangle) Registering interface..." << std::endl;
    std::unique_ptr p = std::make_unique<T>();
    interfaces.push_back(std::move(p));
  }

public:
  static void init();
  static void shutdown();

  static void draw(entt::registry& registry, Shader& shader);
  static void end_frame();
  static int draw_calls();

private:
  static void draw_triangle(const TriangleDescriptor& r, Shader& s);
  static void reset_quad_vert_count();
  static void begin_batch();
  static void end_batch();
  static void flush(Shader& shader); // submit for a drawcall

private:
  static constexpr int max_quad = 10000;
  static constexpr int max_quad_vert_count = max_quad * 3;
  static constexpr int max_quad_index_count = max_quad * 3;
};

} // namespace triangle_renderer

} // namespace engine