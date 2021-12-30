// main.cpp

#if defined(__EMSCRIPTEN__)
#include <SDL.h>
#include <SDL_opengles2.h>
#include <emscripten.h>
#else  // end emscripten
// #include <SDL.h>
// #define IMGUI_IMPL_OPENGL_LOADER_GLEW 1
#endif // end windows

// #include <backends/imgui_impl_opengl3.h>
// #include <backends/imgui_impl_sdl.h>
#include <imgui.h>
// #include <imgui_internal.h>
// #if defined(IMGUI_IMPL_OPENGL_LOADER_GLEW)
// #include <GL/glew.h>
// #endif

#include <iostream>
#include <stdio.h>

// For clarity, our main loop code is declared at the end.
void
main_loop(void* arg)
{
  IM_UNUSED(arg); // do nothing with it

  printf("main loop...");
}

int
main(int argc, char* argv[])
{
  IM_UNUSED(argc);
  IM_UNUSED(argv);

#if defined(__EMSCRIPTEN__)
  std::cout << "Hello, emscripten!" << std::endl;
  // This function call won't return, and will engage in an infinite loop,
  // processing events from the browser, and dispatching them.
  emscripten_set_main_loop_arg(main_loop, NULL, 0, true);
#else
  while (1)
    main_loop(nullptr);
#endif

  exit(0);
}
