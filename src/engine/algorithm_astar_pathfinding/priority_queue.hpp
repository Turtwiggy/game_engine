#pragma once

#include <utility>
#include <vector>

namespace game2d {

template<class T>
class PriorityQueue
{
private:
  std::vector<std::pair<T, int>> elements;

public:
  [[nodiscard]] inline size_t size() const { return elements.size(); }

  inline void enqueue(const T& item, const int& priority) { elements.push_back({ item, priority }); }

  [[nodiscard]] inline T dequeue()
  {
    int best_index = 0;
    for (int i = 0; auto [item, priority] : elements) {
      if (priority < elements[best_index].second)
        best_index = i;
      ++i;
    }

    T best_item = elements[best_index].first;
    elements.erase(elements.begin() + best_index);
    return best_item;
  }
};

} // namespace game2d