// main.cpp : This file contains the 'main' function. Program execution begins and ends there.

#include <stdio.h>

#include "game.h"
#undef main //thanks sdl2

//networking

//void render_ui(sf::RenderWindow& window, game_state& state, network_settings& settings)
//{
//    ImGui::Begin("Networking UI");
//
//    if (networking_thread == nullptr)
//    {
//        if (ImGui::Button("START AS SERVER"))
//        {
//            printf("starting as server");
//            InitSteamDatagramConnectionSockets();
//            settings.is_server = true;
//            quit = false;
//
//            networking_thread = new std::thread([&]()
//                {
//                    settings.server.Run((uint16)settings.port);
//                });
//        };
//
//        if (ImGui::Button("START AS CLIENT"))
//        {
//            printf("starting as client");
//            InitSteamDatagramConnectionSockets();
//            settings.is_client = true;
//            quit = false;
//
//            networking_thread = new std::thread([&]()
//                {
//                    settings.client.Run(settings.addrServer);
//                });
//        };
//    }
//    else
//    {
//        ImGui::Text("You are a server or client");
//
//        //ImGui::InputText();
//
//        if (ImGui::Button("Quit"))
//        {
//            //stop networking
//            quit = true;
//            //networking_thread->join();
//            ShutdownSteamDatagramConnectionSockets();
//
//            delete networking_thread;
//        }
//    }
//
//    ImGui::End();
//}

//rendering physics

////called as fast as possible
//void render(sf::RenderWindow& window, game_state& state, network_settings& settings)
//{
//    //physics stuff
//    //int num_objects = state.physics.dynamicsWorld->getNumCollisionObjects();
//    ////printf("num_objects: %i", num_objects);
//
//    ////print positions of all objects
//    //for (int j = num_objects - 1; j >= 0; j--)
//    //{
//    //    btCollisionObject* obj = state.physics.dynamicsWorld->getCollisionObjectArray()[j];
//    //    btRigidBody* body = btRigidBody::upcast(obj);
//    //    btTransform trans;
//    //    if (body && body->getMotionState())
//    //    {
//    //        body->getMotionState()->getWorldTransform(trans);
//    //    }
//    //    else
//    //    {
//    //        trans = obj->getWorldTransform();
//    //    }
//    //    //printf("world pos object %d = %f,%f,%f\n", j, float(trans.getOrigin().getX()), float(trans.getOrigin().getY()), float(trans.getOrigin().getZ()));
//    //}
//
//    render_ui(window, state, settings);
//}

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
