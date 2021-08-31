#include "gendata.hpp"
#include "lavastone.hpp"
#include <boost/archive/binary_iarchive.hpp>
#include <boost/archive/binary_oarchive.hpp>
#include <boost/archive/text_iarchive.hpp>
#include <boost/archive/text_oarchive.hpp>
#include <boost/serialization/string.hpp>
#include <boost/serialization/unordered_map.hpp>
#include <chrono>
#include <fstream>
#include <iostream>
#include <map>
#include <sstream>
#include <stdlib.h>
#include <string>
#include <vector>

const int n_trials = 1;

template <typename T> double avg_ms_boost_deserialize(int num_trials, T map) {
  unsigned long long total_ns = 0;
  for (size_t j = 0; j < num_trials; j++) {
    std::stringstream ss;
    boost::archive::binary_oarchive oarch(ss);
    oarch << map;
    auto start = std::chrono::steady_clock::now();
    T new_map;
    boost::archive::binary_iarchive iarch(ss);
    iarch >> new_map;
    auto end = std::chrono::steady_clock::now();
    total_ns +=
        std::chrono::duration_cast<std::chrono::nanoseconds>(end - start)
            .count();
  }
  return ((double)total_ns) / ((double)num_trials) / 1000000.;
}

template <typename T> double avg_ms_lava_deserialize(int num_trials, T map) {
  unsigned long long total_ns = 0;
  for (size_t j = 0; j < num_trials; j++) {
    auto packed = Pack(&map);
    auto start = std::chrono::steady_clock::now();
    T new_map;
    Unpack(packed, &new_map);
    auto end = std::chrono::steady_clock::now();
    total_ns +=
        std::chrono::duration_cast<std::chrono::nanoseconds>(end - start)
            .count();
  }
  return ((double)total_ns) / ((double)num_trials) / 1000000.;
}

template <typename T> void benchmark_disk(T &map) {
  {
    std::cout << "serializing map to disk with boost\n";
    auto start = std::chrono::steady_clock::now();
    for (size_t j = 0; j < n_trials; j++) {
      std::ofstream ofs("boost.map", std::ios::out | std::ios::binary);
      boost::archive::binary_oarchive oarch(ofs);
      oarch << map;
      ofs.close();
    }
    auto end = std::chrono::steady_clock::now();
    std::cout << "stored map with " << map.size() << " records in "
              << std::chrono::duration_cast<std::chrono::microseconds>(end -
                                                                       start)
                         .count() /
                     n_trials
              << " µs\n";
  }
  {
    std::cout << "loading with boost\n";
    T new_map;
    auto start = std::chrono::steady_clock::now();
    for (size_t j = 0; j < n_trials; j++) {
      std::ifstream ifs("boost.map", std::ios::in | std::ios::binary);
      boost::archive::binary_iarchive iarch(ifs);
      iarch >> new_map;
      ifs.close();
    }
    auto end = std::chrono::steady_clock::now();
    std::cout << "loaded map with " << new_map.size() << " records in "
              << std::chrono::duration_cast<std::chrono::microseconds>(end -
                                                                       start)
                         .count() /
                     n_trials
              << " µs\n";
    std::cout << (map == new_map) << std::endl;
  }

  {
    std::cout << "serializing to disk with lavastone\n";
    auto start = std::chrono::steady_clock::now();
    for (size_t j = 0; j < n_trials; j++) {
      pack_to_file(&map, "lavastone.map");
    }
    auto end = std::chrono::steady_clock::now();
    std::cout << "stored map with " << map.size() << " records in "
              << std::chrono::duration_cast<std::chrono::microseconds>(end -
                                                                       start)
                         .count() /
                     n_trials
              << " µs\n";
  }
  {
    std::cout << "loading with lavastone\n";
    T new_map;
    auto start = std::chrono::steady_clock::now();
    for (size_t j = 0; j < n_trials; j++) {
      unpack_from_file("lavastone.map", &new_map);
    }
    auto end = std::chrono::steady_clock::now();
    std::cout << "loaded map with " << new_map.size() << " records in "
              << std::chrono::duration_cast<std::chrono::microseconds>(end -
                                                                       start)
                         .count() /
                     n_trials
              << " µs\n";
    std::cout << (map == new_map) << std::endl;
  }
}

template <typename T> void benchmark(T &map) {
  {
    std::cout << "serializing map with boost\n";
    auto start = std::chrono::steady_clock::now();
    for (size_t j = 0; j < n_trials; j++) {
      std::stringstream ss;
      boost::archive::binary_oarchive oarch(ss);
      oarch << map;
    }
    auto end = std::chrono::steady_clock::now();
    std::cout << "stored map with " << map.size() << " records in "
              << std::chrono::duration_cast<std::chrono::microseconds>(end -
                                                                       start)
                         .count() /
                     n_trials
              << " µs\n";
  }
  {
    std::cout << "loading with boost\n";
    T new_map;
    std::stringstream ss;
    boost::archive::binary_oarchive oarch(ss);
    oarch << map;

    auto start = std::chrono::steady_clock::now();
    for (size_t j = 0; j < n_trials; j++) {
      boost::archive::binary_iarchive iarch(ss);
      iarch >> new_map;
    }
    auto end = std::chrono::steady_clock::now();
    std::cout << "loaded map with " << new_map.size() << " records in "
              << std::chrono::duration_cast<std::chrono::microseconds>(end -
                                                                       start)
                         .count() /
                     n_trials
              << " µs\n";
    std::cout << (map == new_map) << std::endl;
  }

  {
    std::cout << "serializing with lavastone\n";
    auto start = std::chrono::steady_clock::now();
    for (size_t j = 0; j < n_trials; j++) {
      auto packed = Pack(&map);
    }
    auto end = std::chrono::steady_clock::now();
    std::cout << "stored map with " << map.size() << " records in "
              << std::chrono::duration_cast<std::chrono::microseconds>(end -
                                                                       start)
                         .count() /
                     n_trials
              << " µs\n";
  }
  {
    std::cout << "loading with lavastone\n";
    auto packed = Pack(&map);
    T new_map;
    auto start = std::chrono::steady_clock::now();
    for (size_t j = 0; j < n_trials; j++) {
      Unpack(packed, &new_map);
    }
    auto end = std::chrono::steady_clock::now();
    std::cout << "loaded map with " << new_map.size() << " records in "
              << std::chrono::duration_cast<std::chrono::microseconds>(end -
                                                                       start)
                         .count() /
                     n_trials
              << " µs\n";
    std::cout << (map == new_map) << std::endl;
  }
}

template <typename T>
std::unordered_map<int, T> random_map(int n, int k,
                                      std::function<T(int)> random_value) {
  std::unordered_map<int, T> map;
  for (int i = 0; i < n; i++) {
    map[std::rand()] = random_value(k - 1);
  }
  return map;
}

void fill_map(int &map, int n) { map = std::rand(); }

template <typename T> void fill_map(std::unordered_map<int, T> &map, int n) {
  for (int i = 0; i < n; i++) {
    fill_map(map[std::rand()], n);
  }
}

std::string random_string() {
  return random_recipe().title + ", " + random_recipe().author_location;
}

int main() {
  if (false) {
    std::unordered_map<int, int> map;
    std::unordered_map<int, std::string> int_string_map;
    std::unordered_map<std::string, int> string_int_map;
    std::unordered_map<std::string, std::string> string_string_map;
    std::unordered_map<
        int, std::unordered_map<
                 int, std::unordered_map<int, std::unordered_map<int, int>>>>
        deep_map;
    std::cout << "preparing maps\n";
    for (size_t i = 0; i < 1000000; i++) {
      map[std::rand()] = std::rand();

      int_string_map[std::rand()] = random_recipe().title;
      string_int_map[random_recipe().title] = std::rand();
      string_string_map[random_recipe().title] = random_recipe().author;
    }
    fill_map(deep_map, 10);

    std::cout << "benchmarking int->int map\n";
    benchmark(map);
    std::cout << "\n\n";
    benchmark_disk(map);
    std::cout << "\n\n";

    std::cout << "benchmarking int->string map\n";
    benchmark(int_string_map);
    std::cout << "\n\n";
    benchmark_disk(int_string_map);
    std::cout << "\n\n";

    std::cout << "benchmarking string->int map\n";
    benchmark(string_int_map);
    std::cout << "\n\n";
    benchmark_disk(string_int_map);
    std::cout << "\n\n";

    std::cout << "benchmarking string->string map\n";
    benchmark(string_string_map);
    std::cout << "\n\n";
    benchmark_disk(string_string_map);
    std::cout << "\n\n";

    std::cout << "benchmarking deep map\n";
    benchmark(deep_map);
    std::cout << "\n\n";
    benchmark_disk(deep_map);
    std::cout << "\n\n";
  }

  std::ofstream ofs("deserialization_times.csv");
  ofs << "num_records,"
      << "int_int_boost_ms," <<
      //          "int_string_boost_ns," <<
      //          "string_int_boost_ms," <<
      //          "string_string_boost_ms," <<
      "int_int_lava_ms," <<
      //          "int_string_lava_ms," <<
      //          "string_int_lava_ms," <<
      //          "string_string_lava_ms" <<
      "\n";

  std::vector<int> trial_sizes;
  {
    int num_records = 1;
    num_records *= 1.1;
    for (int i = 0; i < 150; i++) {
      trial_sizes.push_back(num_records);
      num_records++;
      num_records *= 1.1;
    }
  }

  for (int num_records : trial_sizes) {

    std::unordered_map<int, int> int_int_map;
    std::unordered_map<int, std::string> int_string_map;
    std::unordered_map<std::string, int> string_int_map;
    std::unordered_map<std::string, std::string> string_string_map;
    while (int_int_map.size() < num_records) {
      int_int_map[std::rand()] = std::rand();
    }
    //    while (int_string_map.size() < num_records) {
    //      int_string_map[std::rand()] = random_string();
    //    }
    //    while (string_int_map.size() < num_records) {
    //      string_int_map[random_string()] = std::rand();
    //    }
    //    while (string_string_map.size() < num_records) {
    //      string_string_map[random_string()] = random_string();
    //    }

    std::cout << "num_records = " << num_records << "\n";
    const int num_trials = 3 + (10000 / num_records);
    ofs << num_records << ","
        << avg_ms_boost_deserialize(num_trials, int_int_map) << "," <<
        //           avg_ms_boost_deserialize(num_trials, int_string_map) << ","
        //           << avg_ms_boost_deserialize(num_trials, string_int_map) <<
        //           "," << avg_ms_boost_deserialize(num_trials,
        //           string_string_map) << "," <<
        avg_ms_lava_deserialize(num_trials, int_int_map) << "\n"; //"," <<
    //           avg_ms_lava_deserialize(num_trials, int_string_map) << "," <<
    //           avg_ms_lava_deserialize(num_trials, string_int_map) << "," <<
    //           avg_ms_lava_deserialize(num_trials, string_string_map) << "\n";

    //    num_records *= 2;
  }
  ofs.close();
}
