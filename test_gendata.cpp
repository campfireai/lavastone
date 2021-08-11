#include "gendata.hpp"
#include <fstream>
#include <iostream>

int main(int argc, char *argv[]) {
  if (argc != 2) {
    std::cerr << "usage:\n"
              << "./test_gendata NUM_RECORDS\n";
    exit(1);
  }

  size_t num_records = std::stoi(argv[1]);

  std::string fname = "data.csv";
  std::cout << "writing " << num_records << " records to " << fname << "\n";

  std::ofstream outfile;
  outfile.open(fname);
  outfile << "title,author,author_location,num_likes"
          << "\n";
  for (auto r : random_recipes(num_records)) {
    outfile << "\"" << r.title << "\",\"" << r.author << "\",\""
            << r.author_location << "\",\"" << r.num_likes << "\"\n";
  }
  outfile.close();
}
