#include "gendata.hpp"
#include "lavapack.hpp"
// this must be invoked once to allow (de)serialization of this struct type
LAVAPACK_ADAPT_STRUCT(recipe, title, author, author_location, num_likes);
#include "lavastone.hpp"
#include <chrono>
#include <iostream>
#include <string>

#define demand(cond, str)                                                      \
  if (!(cond)) {                                                               \
    std::cout << str << std::endl;                                             \
    exit(1);                                                                   \
  }

#define timeit(code, str)                                                      \
  {                                                                            \
    auto start = std::chrono::steady_clock::now();                             \
    code;                                                                      \
    auto end = std::chrono::steady_clock::now();                               \
    std::cout << str                                                           \
              << std::chrono::duration_cast<std::chrono::microseconds>(end -   \
                                                                       start)  \
                     .count()                                                  \
              << "\n";                                                         \
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
  std::unordered_map<std::string, std::unordered_map<int, std::vector<recipe>>>
      name_to_num_likes_to_recipes;
  lava::Ref<decltype(name_to_num_likes_to_recipes)>
      name_to_num_likes_to_recipes_ref;

  timeit(
      for (auto r
           : recipes) {
        name_to_num_likes_to_recipes[r.author][r.num_likes].push_back(r);
      },
      "random_insert_memory_us=")

      timeit(
          for (auto r
               : recipes) {
            name_to_num_likes_to_recipes_ref[r.author][r.num_likes].push_back(
                r);
          },
          "random_insert_disk_us=")

          lava::Ref<decltype(name_to_num_likes_to_recipes)>
              name_to_num_likes_to_recipes_ref_bulk;
  timeit(name_to_num_likes_to_recipes_ref_bulk = name_to_num_likes_to_recipes;
         , "copy_memory_to_disk_us=") decltype(name_to_num_likes_to_recipes)
      recovered_1;
  timeit(recovered_1 = name_to_num_likes_to_recipes_ref;
         , "copy_disk_to_memory_us=") decltype(name_to_num_likes_to_recipes)
      recovered_2 = name_to_num_likes_to_recipes_ref_bulk;
  assert(recovered_1 == recovered_2 &&
         recovered_2 == name_to_num_likes_to_recipes);

  size_t total_chars = 0;

  timeit(
      for (size_t i = 0; i < num_records; i++) {
        size_t j = std::rand() % num_records;
        std::vector<recipe> some_recipes =
            name_to_num_likes_to_recipes[recipes[j].author]
                                        [recipes[j].num_likes];
        for (auto r : some_recipes)
          total_chars += r.title.size();
      },
      "random_access_memory_us=")

          timeit(
              for (size_t i = 0; i < num_records; i++) {
                size_t j = std::rand() % num_records;
                std::vector<recipe> some_recipes =
                    name_to_num_likes_to_recipes_ref[recipes[j].author]
                                                    [recipes[j].num_likes];
                for (auto r : some_recipes)
                  total_chars += r.title.size();
              },
              "random_access_disk_us=") std::cout
      << "total_chars = " << total_chars << "\n";
}
