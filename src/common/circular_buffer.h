#pragma once

#include <array>
#include <cmath>
#include <algorithm>

namespace fightinggame
{
    struct circular_buffer
    {
    public:

        void add_next(const float next)
        {
            buffer[next_index] = next;

            next_index += 1;
            next_index %= buffer_size;

            if (populated_elements == buffer_size)
                return;
            populated_elements += 1.;
        }

        float sum()
        {
            float sum = 0;
            for (auto i = 0; i < buffer_size; i += 1)
            {
                sum += buffer[i];
            }
            return sum;
        }

        float average()
        {
            if (populated_elements == 0)
                return 0;
            else
                return sum() / populated_elements;
        }

        float get(uint8_t index)
        {
            return buffer[index];
        }

    private:

        uint8_t next_index = 0;
        float populated_elements = 0;

        static constexpr uint8_t buffer_size = 250; //note if >255 then uint8_t is not big enough

        std::array<float, buffer_size> buffer = { 0 };
    };
}
