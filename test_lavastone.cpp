#include "gendata.hpp"
#include <iostream>

int main(int argc, char *argv[]) {

  size_t num_records;
  if (argc != 2) {
    num_records = 10000;
    std::cout << "using default of " << num_records << " records\n";
  } else {
    num_records = std::stoi(argv[1]);
    std::cout << "using " << num_records << " records\n";
  }
  auto recipes = random_recipes(num_records);

}
