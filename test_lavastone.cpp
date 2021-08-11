#include "gendata.hpp"
#include "lavastone.hpp"
#include <chrono>
#include <iostream>
#include <string>

#define demand(cond, str)                                                      \
  if (!(cond)) {                                                               \
    std::cout << str << std::endl;                                             \
    exit(1);                                                                   \
  }

void test_string(std::string foo) {
  std::cout << "\nstoring string " << foo << "\n";
  pack_to_file(&foo, "foo.string");
  std::string foo_loaded;
  unpack_from_file("foo.string", &foo_loaded);
  std::cout << "loaded string " << foo_loaded << "\n\n";
  demand(foo_loaded == foo, "string pack/unpack error");
}

template <typename T> void test_struct(T foo) {
  std::cout << "\nstoring struct " << foo << "\n";
  pack_to_file(&foo, "foo.struct");
  T foo_loaded;
  unpack_from_file("foo.struct", &foo_loaded);
  std::cout << "loaded struct " << foo_loaded << "\n\n";
  demand(foo_loaded == foo, "struct pack/unpack error");
}

template <typename T> void test_vector(std::vector<T> foo) {
  auto start = std::chrono::steady_clock::now();
  std::cout << "storing vector\n";
  pack_to_file(&foo, "foo.vector");
  auto end = std::chrono::steady_clock::now();
  std::cout << "stored vector with " << foo.size() << " records in "
            << std::chrono::duration_cast<std::chrono::microseconds>(end -
                                                                     start)
                   .count()
            << " µs\n";

  std::vector<T> foo_loaded;

  start = std::chrono::steady_clock::now();
  unpack_from_file("foo.vector", &foo_loaded);
  end = std::chrono::steady_clock::now();
  std::cout << "loaded vector with " << foo.size() << " records in "
            << std::chrono::duration_cast<std::chrono::microseconds>(end -
                                                                     start)
                   .count()
            << " µs\n";
  demand(foo_loaded == foo, "vector pack/unpack error");
}

template <typename T> void test_fixed_width(T foo) {
  std::cout << "storing value " << foo << " of fixed-width type\n";
  pack_to_file(&foo, "foo.fixed_width");
  T foo_loaded;
  unpack_from_file("foo.fixed_width", &foo_loaded);
  std::cout << "loaded value " << foo_loaded << " of fixed-width type\n";
  demand(foo_loaded == foo, "fixed-width pack/unpack error");
}

template <typename T1, typename T2>
void test_unordered_map(std::unordered_map<T1, T2> foo) {
  std::cout << "storing unordered map\n";
  pack_to_file(&foo, "foo.unordered_map");
  std::unordered_map<T1, T2> foo_loaded;
  unpack_from_file("foo.unordered_map", &foo_loaded);
  std::cout << "loaded unordered map\n";
  demand(foo_loaded == foo, "unordered_map pack/unpack error");
}

template <typename T> void test_set(std::set<T> foo) {
  std::cout << "storing set\n";
  pack_to_file(&foo, "foo.set");
  std::set<T> foo_loaded;
  unpack_from_file("foo.set", &foo_loaded);
  std::cout << "loaded set\n";
  demand(foo_loaded == foo, "set pack/unpack error");
}

template <typename T1, typename T2> void test_map(std::map<T1, T2> foo) {
  std::cout << "storing map\n";
  pack_to_file(&foo, "foo.map");
  std::map<T1, T2> foo_loaded;
  unpack_from_file("foo.map", &foo_loaded);
  std::cout << "loaded map\n";
  demand(foo_loaded == foo, "map pack/unpack error");
}

// this must be invoked once to allow (de)serialization of this struct type
LAVASTONE_ADAPT_STRUCT(recipe, title, author, author_location, num_likes);

int main(int argc, char *argv[]) {

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
  std::unordered_map<std::string, size_t> num_recipes_by_author;
  // build a fake index to test map serialization
  for (auto r : recipes)
    num_recipes_by_author[r.author + " | " + r.author_location]++;

  std::map<size_t, std::set<std::string>> authors_by_num_recipes;
  for (auto it : num_recipes_by_author) {
    authors_by_num_recipes[it.second].insert(it.first);
  }

  std::set<int> a_set = {1, 2, 3, 4, 5};

  test_string(recipes.at(0).title);
  test_struct(recipes.at(0));
  test_vector(recipes);
  test_fixed_width(recipes.size());
  test_set(a_set);
  test_unordered_map(num_recipes_by_author);
  test_map(authors_by_num_recipes);
}
