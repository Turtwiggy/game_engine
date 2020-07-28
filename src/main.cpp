// main.cpp : This file contains the 'main' function. Program execution begins and ends there.

#include <stdio.h>

#include "game.h"
#undef main //thanks sdl2

int main()
{
    std::cout << "============================================================="
        "=================\n"
        "  HONK ENGINE   \n"
        "============================================================="
        "=================\n"
        "\n";

    fightinggame::Game game;
    game.run();

    return 1;
}
