#include "app/application.hpp"
#include "app/io.hpp"
#include "opengl/util.hpp"
using namespace engine;

#if defined(__EMSCRIPTEN__)
#include <emscripten.h>
#endif

// other libs
#include <entt/entt.hpp>
#include <imgui.h>

// std lib
#include <chrono>

// TEST
#include "example_function.hpp"

int
main(int argc, char* argv[])
{
  IM_UNUSED(argc);
  IM_UNUSED(argv);

  printf("Hello, World!");

  game2d::function_returns_true();

  return 0;
}