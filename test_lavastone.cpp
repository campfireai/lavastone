#include "gendata.hpp"
#include "lavapack.hpp"
// this must be invoked once to allow (de)serialization of this struct type
LAVASTONE_ADAPT_STRUCT(recipe, title, author, author_location, num_likes);
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

  // specify ID to access persistent container across runs of the program
  lava::Ref<std::vector<recipe>> persistent_vector_ref(0);
  // persistent_vector_ref = lava::Ref<std::vector<recipe>>(0);

  // if ID is not specified an ID will be chosen automatically
  lava::Ref<std::vector<recipe>> vector_ref;

  std::cout << "vector_ref.size() = " << vector_ref.size() << "\n";
  std::cout << "persistent_vector_ref.size() = " << persistent_vector_ref.size() << "\n";
  vector_ref = random_recipes(num_records);
  for (auto r: random_recipes(num_records)) {
    vector_ref.push_back(r);
    persistent_vector_ref.push_back(r);
  }
  recipe final_elem = vector_ref.at(vector_ref.size()-1);
  recipe final_persistent_elem = persistent_vector_ref.at(persistent_vector_ref.size()-1);
  std::cout << "final recipe title = " << final_elem.title << "\n";
  std::cout << "final persistent recipe title = " << final_persistent_elem.title << "\n";


  lava::Ref<std::string> string_ref;
  string_ref = "456def";
  std::string loaded_string_ref = string_ref;
  std::cerr << "string_ref = " << loaded_string_ref << "\n";

  lava::Ref<std::vector<std::vector<std::string>>> vecvecstring_ref;
  lava::Ref<std::vector<std::string>> vecstring_ref;
  std::vector<std::string> vecstring;

  vecstring.push_back("hello");
  vecstring.push_back("RAM");
  vecstring_ref.push_back("hello");
  vecstring_ref.push_back("DISK");
  vecvecstring_ref.push_back(vecstring);
  vecvecstring_ref.push_back(vecstring_ref);
  vecvecstring_ref.push_back({});
  vecvecstring_ref.at(2).push_back("hello");
  vecvecstring_ref.at(2).push_back("WORLD");
  vecvecstring_ref.at(2).push_back("!");
  std::cout << "vecvecstring_ref.size() = " << vecvecstring_ref.size() << "\n";
  for (size_t i=0; i<vecvecstring_ref.size(); i++) {
    for (size_t j=0; j<vecvecstring_ref.at(i).size(); j++) {
      std::cout << "vecvecstring_ref.at("<<i<<").at("<<j<<") = " << std::string(vecvecstring_ref.at(i).at(j)) << "\n";
    }
  }

  lava::Ref<std::unordered_map<std::string, std::string>> mapstring_ref;
  mapstring_ref["pudding"] = "delicious";
  std::cout << "mapstring_ref[\"pudding\"] = " << std::string(mapstring_ref["pudding"]) << "\n";
  std::unordered_map<std::string, std::string> loaded_mapstring_ref = mapstring_ref;
  std::cout << "loaded_mapstring_ref.size() = " << loaded_mapstring_ref.size() << "\n";
  std::cout << "loaded_mapstring_ref[\"pudding\"] = " << loaded_mapstring_ref["pudding"] << "\n";

  lava::Ref<std::unordered_map<size_t, size_t>> mapint_ref;
  mapint_ref[12] = 24;
  std::cout << "mapint_ref[12] = " << mapint_ref[12] << "\n";


  lava::Ref<std::unordered_map<std::string, std::unordered_map<std::string, std::string>>> deep_map_ref;
  deep_map_ref["abc"]["123"] = "hello world";
  std::cerr<<"deep_map_ref.at(\"abc\").at(\"123\") = " << std::string(deep_map_ref.at("abc").at("123")) << "\n";
  std::cerr << "deep_map_ref.at(\"abc\").size() = " << deep_map_ref.at("abc").size() << "\n";

  lava::Ref<
    std::unordered_map<
      std::string,
      std::unordered_map<
        size_t,
        std::vector<
          std::unordered_map<
            int,
            std::string
          >
        >
      >
    >
  > deep_datastructure;

  deep_datastructure["abc"][123].push_back({});
  deep_datastructure["abc"][123].at(0)[99] = std::string(string_ref);
  deep_datastructure["abc"][123].push_back({{42, "oh YEAH"}});

  std::cerr << "deep_datastructure[\"abc\"][123].at(0).at(99) = " << std::string(deep_datastructure["abc"][123].at(0).at(99)) << "\n";
  std::cerr << "deep_datastructure[\"abc\"][123].at(0).at(99) = " << std::string(deep_datastructure["abc"][123].at(1).at(42)) << "\n";
  std::cerr << "deep_datastructure.at(\"abc\")(123).at(0).at(99) = " << std::string(deep_datastructure.at("abc").at(123).at(1).at(42)) << "\n";

  std::unordered_map<
    std::string,
    std::unordered_map<
      size_t,
      std::vector<
        std::unordered_map<
          int,
          std::string
        >
      >
    >
  > loaded_deep_datastructure = deep_datastructure;
  assert(loaded_deep_datastructure.size() == deep_datastructure.size());

  std::cout << "all tests passed\n";
}
