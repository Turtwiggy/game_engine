#pragma once

#include <array>
#include <cmath>
#include <algorithm>

namespace fightingengine
{
    template <typename T, uint8_t N>
    struct CircularBuffer
    {
        static constexpr uint8_t _bufferSize = N;
        T _values[_bufferSize] = {};
        uint8_t _offset = 0;
        uint8_t _populated_elements = 0;

        [[nodiscard]] T back() const { return _values[_offset]; }
        void push_back(T value)
        {
            _values[_offset] = value;
            _offset = (_offset + 1u) % _bufferSize;

            if (_populated_elements == _bufferSize)
                return;
            _populated_elements += 1u;
        }

        T sum()
        {
            T sum = 0;
            for (auto i = 0; i < _bufferSize; i += 1)
            {
                sum += _values[i];
            }
            return sum;
        }

        T average()
        {
            if (_populated_elements == 0)
                return 0;
            else
                return sum() / _populated_elements;
        }
    };
}
