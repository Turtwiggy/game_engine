#pragma once

namespace fightinggame {

    //http://bits.stephan-brumme.com/roundUpToNextPowerOfTwo.html
    unsigned int next_power_of_two(unsigned int x)
    {
        x--;
        x |= x >> 1;  // handle  2 bit numbers
        x |= x >> 2;  // handle  4 bit numbers
        x |= x >> 4;  // handle  8 bit numbers
        x |= x >> 8;  // handle 16 bit numbers
        x |= x >> 16; // handle 32 bit numbers
        x++;
        return x;
    }

}
