#include "lavastone.hpp"
#include <iostream>
#include <string>
#include <unordered_map>
#include <vector>

using namespace std;
int main() {
  // always run this before constructing lava::Ref<>'s:
  lava::init();

  // here is an ordinary memory-backed map:
  unordered_map<string, int> a_map;
  a_map["hello"] = 12;
  cout << a_map["hello"] << "\n";

  // let's make it disk-backed by wrapping in lava::Ref<...>:
  lava::Ref<unordered_map<string, int>> cool_map_on_disk;
  cool_map_on_disk["world"] = 13;
  cout << cool_map_on_disk["world"] << "\n";
  // things works pretty much the same way!

  // lava::Ref also supports vectors and nested types:
  lava::Ref<vector<unordered_map<string, int>>> vector_of_maps_on_disk;

  // disk-backed containers play nice with each other:
  vector_of_maps_on_disk.push_back(cool_map_on_disk);
  // and also with memory-backed containers:
  vector_of_maps_on_disk.push_back(a_map);

  // assignment works as expected:
  vector_of_maps_on_disk.at(0) = a_map;
  vector_of_maps_on_disk.at(1) = cool_map_on_disk;
  a_map = vector_of_maps_on_disk.at(0);

  cout << vector_of_maps_on_disk.at(0).at("hello") << "\n";
  cout << vector_of_maps_on_disk.at(1).at("world") << "\n";
}
