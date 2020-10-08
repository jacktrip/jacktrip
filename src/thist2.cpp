// clang++ -g -std=c++14 -I/opt/local/include  -L/opt/local/lib  thist2.cpp   -o thist2

#include <iostream>
#include <vector>
#include <algorithm>
#include <string>
#include <random>
#include <map>

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

// set up for one die:
static const int distMin = 1;
static const int distMax = 6;

std::vector<int> roll(int times) {
  std::vector<int> randomInts;
  randomInts.reserve(times);

  std::random_device seeder;
  std::mt19937 engine(seeder());
  std::uniform_int_distribution<int> dist(distMin, distMax);

  while (times > 0) {
    randomInts.push_back(dist(engine));
    --times;
  }
  return randomInts;
}

std::map<int, int> histogram_calculate(int times) {
  std::vector<int> random_numbers = roll(times);
  std::map<int, int> cnt_hashmap;
  for (int i = distMin; i <= distMax; ++i) {
    cnt_hashmap[i] = 0;
  }
  for (auto iter : random_numbers) {
    cnt_hashmap[iter] += 1;
  }
  return cnt_hashmap;
}

std::string histogram_draw(int times) {
  std::vector<std::string> ret_vec;
  std::map<int, int> histogram = histogram_calculate(times);
  for (int i = 1; i <= histogram.size(); ++i) {
    std::string rows = "";
    int hi = histogram[i];
    std::string istr = std::to_string(i);
    std::string histr = std::to_string(hi);
    std::string row = "["+istr+"]="+histr+":";
    for (int j=0; j<histogram[i]; j++) {
      row += "#";
    }
    rows += row + "\n";
    // rows += std::to_string(i);
    ret_vec.push_back(rows);
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
