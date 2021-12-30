// main.cpp

#if defined(__EMSCRIPTEN__)
#include <emscripten.h>
#endif
#include <imgui.h>

#include <iostream>
#include <stdio.h>

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
  // This function call won't return, and will engage in an infinite loop,
  // processing events from the browser, and dispatching them.
  emscripten_set_main_loop_arg(main_loop, NULL, 0, true);
#else
  while (1)
    main_loop(nullptr);
#endif

  exit(0);
}
