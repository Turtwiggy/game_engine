#pragma once

#include <GL/glew.h>

namespace fightinggame {

#define CHECK_OPENGL_ERROR(x) GLenum err_##x = glGetError(); \
                if (GL_NO_ERROR != err_##x){ \
                printf("\n ~~ ERROR ~~ %i: %s \n", x, gl_error_to_string(err_##x)); }

    //http://bits.stephan-brumme.com/roundUpToNextPowerOfTwo.html
    unsigned int next_power_of_two(unsigned int x);

    const char* gl_error_to_string(GLenum err);

}
