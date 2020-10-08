// clang++ -std=c++14 -I/opt/local/include  -L/opt/local/lib  thist2.cpp   -o thist2

#include <iostream>
#include <vector>
#include <algorithm>
#include <string>
#include <random>
#include <map>

std::vector<int> roll(int times) {
  std::vector<int> rand;
  rand.reserve(times);

  std::random_device seeder;
  std::mt19937 engine(seeder());
  std::uniform_int_distribution<int> dist(1, 6);

  while (times > 0) {
    rand.push_back(dist(engine));
    --times;
  }
  return rand;
}

std::vector<int> roll0(int times) {
  std::vector<int> rand;

  while (times > 0) {
    std::random_device seeder;
    std::mt19937 engine(seeder());
    std::uniform_int_distribution<int> dist(1, 6);
    rand.push_back(dist(engine));

    --times;
  }
  return rand;
}

std::map<int, int> histogram_calculate(int times) {
  std::vector<int> random_numbers = roll(times);

  std::map<int, int> cnt_hashmap;
  auto max_element = 6;

  for (int i = 1; i <= max_element; ++i) {
    cnt_hashmap[i] = 0;
  }

  for (auto iter = random_numbers.begin(); iter != random_numbers.end(); ++iter) {
    cnt_hashmap[*iter] += 1;
  }

  return cnt_hashmap;
}

std::string histogram_draw(int times) {
  std::vector<std::string> ret_vec;
  std::map<int, int> histogram = histogram_calculate(times);

  for (int i = 1; i <= histogram.size(); ++i)
    {
      std::string to_add = "";

      if (histogram[i] > 0)
        {
	  to_add = std::to_string(histogram[i]);
	  std::string column = "\n";
	  int j = 0;

	  while (j <= histogram[i])
            {
	      column += "#";
	      //                column += "\n";

	      ++j;
            }

	  to_add += column;
        }

      to_add += "--";
      to_add += std::to_string(i);

      ret_vec.push_back(to_add);

    }

  std::string finalize = "";
  for (auto &str : ret_vec) {
    finalize += str;
  }
  return finalize;
}

int main() {
  std::cout << histogram_draw(10) << std::endl;
  return 0;
}
