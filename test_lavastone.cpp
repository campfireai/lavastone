#include "gendata.hpp"
#include "lavastone.hpp"
#include <iostream>

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
  std::cout << "storing vector\n";
  pack_to_file(&foo, "foo.vector");
  std::vector<T> foo_loaded;
  unpack_from_file("foo.vector", &foo_loaded);
  std::cout << "loaded vector\n";
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

template <typename T1, typename T2> void test_unordered_map(std::unordered_map<T1,T2> foo) {
  std::cout << "storing container type\n";
  pack_to_file(&foo, "foo.container");
  std::unordered_map<T1,T2> foo_loaded;
  unpack_from_file("foo.container", &foo_loaded);
  std::cout << "loaded container type\n";
  demand(foo_loaded == foo, "container pack/unpack error");
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
  for (auto r: recipes)
      num_recipes_by_author[r.author + " | " + r.author_location]++;

  test_string(recipes.at(0).title);
  test_struct(recipes.at(0));
  test_vector(recipes);
  test_fixed_width(recipes.size());
  test_unordered_map(num_recipes_by_author);
}
