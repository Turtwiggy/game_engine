#include "helpers.hpp"

namespace game2d {

std::vector<std::string>
convert_int_to_sprites(int damage)
{
  std::string number_0 = { "NUMBER_0" };
  std::string number_1 = { "NUMBER_1" };
  std::string number_2 = { "NUMBER_2" };
  std::string number_3 = { "NUMBER_3" };
  std::string number_4 = { "NUMBER_4" };
  std::string number_5 = { "NUMBER_5" };
  std::string number_6 = { "NUMBER_6" };
  std::string number_7 = { "NUMBER_7" };
  std::string number_8 = { "NUMBER_8" };
  std::string number_9 = { "NUMBER_9" };

  std::vector<std::string> numbers;

  int number = damage;
  // this iterates over number from right to left.
  // e.g. 1230 will iterate as 0, 3, 2, 1
  while (number > 0) {
    int digit = number % 10;
    number /= 10;

    if (digit == 9)
      numbers.push_back(number_9);
    if (digit == 8)
      numbers.push_back(number_8);
    if (digit == 7)
      numbers.push_back(number_7);
    if (digit == 6)
      numbers.push_back(number_6);
    if (digit == 5)
      numbers.push_back(number_5);
    if (digit == 4)
      numbers.push_back(number_4);
    if (digit == 3)
      numbers.push_back(number_3);
    if (digit == 2)
      numbers.push_back(number_2);
    if (digit == 1)
      numbers.push_back(number_1);
    if (digit == 0)
      numbers.push_back(number_0);
  }

  std::reverse(numbers.begin(), numbers.end());
  return numbers;
}

} // namespace game2d