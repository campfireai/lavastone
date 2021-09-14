#include "gendata.hpp"
#include "lavapack.hpp"
struct reduced_recipe {
  std::string title;
  int author_id;
  int num_likes;
};
LAVAPACK_ADAPT_STRUCT(reduced_recipe, title, author_id, num_likes);
#include "lavastone.hpp"
#include <chrono>
#include <iostream>
#include <string>

#define demand(cond, str)                                                      \
  if (!(cond)) {                                                               \
    std::cout << str << std::endl;                                             \
    exit(1);                                                                   \
  }

int main(int argc, char *argv[]) {

  // test of disk-backed data types
  lava::init();

  size_t num_records;
  if (argc != 2) {
    num_records = 10000;
    std::cout << "using default of " << num_records << " records\n";
  } else {
    num_records = std::stoi(argv[1]);
    std::cout << "using " << num_records << " records\n";
  }
  demand(num_records > 0, "no records to test with!");

  auto recipes = random_recipes(num_records);
  std::unordered_map<std::string, int> author_to_id;
  std::vector<std::string> authors;
  std::vector<reduced_recipe> reduced_recipes;
  for (auto r : recipes) {
    std::string author = r.author + "," + r.author_location;
    int author_id;
    auto it = author_to_id.find(author);
    if (it == author_to_id.end()) {
      author_id = author_to_id.size();
      author_to_id[author] = author_id;
      authors.push_back(author);
    } else {
      author_id = it->second;
    }
    reduced_recipes.push_back(
        {.title = r.title, .author_id = author_id, .num_likes = r.num_likes});
  }
  std::cout << "writing to kvstore via lavastone\n";
  lava::Ref<decltype(reduced_recipes)> reduced_recipes_ref;
  lava::Ref<decltype(authors)> authors_ref;
  reduced_recipes_ref = reduced_recipes;
  authors_ref = authors;
}
