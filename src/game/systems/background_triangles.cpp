// #include "asteroid.hpp"

// #include "engine/maths/maths.hpp"
// #include "game/components/game.hpp"
// #include "game/create_entities.hpp"
// #include "modules/lifecycle/components.hpp"
// #include "modules/physics/components.hpp"
// #include "modules/renderer/components.hpp"

// #include "glm/glm.hpp"
// #include <imgui.h>

// void
// game2d::update_background_triangles_system(entt::registry& r)
// {
//   auto& rnd = r.ctx().at<SINGLETON_ResourceComponent>();
//   auto& eb = r.ctx().at<SINGLETON_EntityBinComponent>();
//   const auto& ri = r.ctx().at<SINGLETON_RendererInfo>();
//   const float dt = ImGui::GetIO().DeltaTime;
//   const auto& view = r.view<AsteroidComponent, VelocityComponent, TransformComponent, EntityTimedLifecycle>();

//   view.each([&gs, &eb, &rnd, &dt, &ri](auto entity, auto& asteroid, auto& vel, auto& transform, auto& lifecycle) {
//     if (!asteroid.initialized) {
//       asteroid.initialized = true;

//       // Start()

//       // .. spawn asteroid at edge of screen
//       const int rnd_screen_edge = static_cast<int>(engine::rand_det_s(rnd.rnd.rng, 0, 4));

//       // .. random starting position
//       float rx = static_cast<float>(ri.viewport_size_render_at.x);
//       float ry = static_cast<float>(ri.viewport_size_render_at.y);
//       float spawn_pos_x = engine::rand_det_s(rnd.rnd.rng, 0.0f, rx);
//       float spawn_pos_y = engine::rand_det_s(rnd.rnd.rng, 0.0f, ry);
//       transform.position.x = static_cast<int>(spawn_pos_x);
//       transform.position.y = static_cast<int>(spawn_pos_y);

//       // .. random starting velocity (but pointed inwards)
//       float spawn_x_vel = 0.0f;
//       float spawn_y_vel = 0.0f;
//       const auto& minv = gs.asteroid_min_vel;
//       const auto& maxv = gs.asteroid_max_vel;
//       vel.x = spawn_x_vel;
//       vel.y = spawn_y_vel;

//       const float rnd_time_alive = engine::rand_det_s(rnd.rnd.rng, 5.0f, 20.0f);
//       lifecycle.time_alive_max = rnd_time_alive;

//       const float rnd_spin_amount = engine::rand_det_s(rnd.rnd.rng, 0.0f, engine::HALF_PI);
//       asteroid.spin_amount = rnd_spin_amount;
//     }

//     // Update()

//     // .. asteroid is off screen
//     // if (transform.position.x >)

//     // .. spin the asteroid
//     // transform.rotation.z += asteroid.spin_amount * dt;
//   });

//   // Asteroid Spawner

//   size_t live_asteroids = r.view<AsteroidComponent>().size();
//   gs.time_since_last_asteroid -= dt;
//   if (gs.time_since_last_asteroid <= 0.0f && live_asteroids <= gs.max_asteroids) {
//     gs.time_since_last_asteroid = gs.time_between_asteroids;
//     create_asteroid(r);

//     // Make game more difficult
//     gs.time_between_asteroids -= 0.01f;
//     gs.time_between_asteroids = glm::max(0.1f, gs.time_between_asteroids);

//     // Make game even more difficult
//     gs.max_asteroids += 1;

//     // MORE DIFFICULT
//     gs.asteroid_min_vel -= 1.0f;
//     gs.asteroid_max_vel += 1.0f;
//   }
// };