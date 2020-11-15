#pragma once

namespace game2d 
{

enum BreakoutGameState {
   GAME_ACTIVE,
   GAME_MENU,
   GAME_WIN
};

class BreakoutGame 
{
public:
    //GameState
    BreakoutGameState state;

public:
    //setup
    void init();

    //game loop
    void input(float delta_time_s);
    void update(float delta_time_s);
    void render();
};

} //namespace game2d