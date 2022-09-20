#pragma once

#include <utility>
#include <vector>

namespace game2d {

// This should maybe be a fixed-size queue
// maybe even a binary heap allocated vector
// could improve when performance needs it
template<class T>
struct PriorityQueue
{
  std::vector<std::pair<T, int>> elements;

public:
  int size() const;
  void enqueue(const T& item, const int& priority);
  T dequeue();
};

} // game2d