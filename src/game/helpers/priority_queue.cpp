#include "priority_queue.hpp"

namespace game2d {

template<class T>
int
PriorityQueue<T>::size() const
{
  return elements.size();
};

template<class T>
void
PriorityQueue<T>::enqueue(const T& item, const int& priority)
{
  elements.push_back({ item, priority });
};

template<class T>
T
PriorityQueue<T>::dequeue()
{
  int best_index = 0;
  for (int i = 0; auto& [first, second] : elements) {
    if (second < elements[best_index])
      best_index = i;
    ++i;
  }

  T best_item = elements[best_index].first;
  elements.erase(elements.begin() + best_index);
  return best_item;
};

} // namespace game2d