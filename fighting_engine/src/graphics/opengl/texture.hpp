#pragma once

#include <string>

namespace fightingengine {

    // https://learnopengl.com/Model-Loading/Model
    // https://learnopengl.com/code_viewer_gh.php?code=src/3.model_loading/1.model_loading/model_loading.cpp
    // https://learnopengl.com/code_viewer_gh.php?code=includes/learnopengl/model.h
    // https://learnopengl.com/code_viewer_gh.php?code=includes/learnopengl/mesh.h

    struct Texture2D {
        unsigned int id;
        std::string type;
        std::string path;
    };

    unsigned int TextureFromFile(const char* file, const std::string& directory, bool gamma = false);

}
