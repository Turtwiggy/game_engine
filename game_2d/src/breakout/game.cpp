
//header
#include "breakout/game.hpp"

//C++ lib headers
#include <iostream>     // std::cout
#include <fstream>      // std::ifstream

namespace game2d {

void load_game_level(GameLevel& level, std::string path, unsigned int level_width, unsigned int level_height) 
{
    level.bricks.clear();

    //https://learnopengl.com/In-Practice/2D-Game/Levels
    //load level from text file
    std::ifstream fstream(path);

    std::vector<std::vector<int>> data;
    if (fstream)
    {

        int tile_code;
        std::string line;
        while(std::getline(fstream, line))
        {   
            std::cout << line << "\n";

            //https://learnopengl.com/In-Practice/2D-Game/Levels
            // std::istringstream sstream(line);
            // std::vector<unsigned int> row;
        }
    } 
    else
    {
        std::cout << "error reading game level file! \n";
    }

};

} //namespace game2d
