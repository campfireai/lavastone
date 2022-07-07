#ifndef __LAVASTONE_HPP__
#define __LAVASTONE_HPP__

#include "lavapack.hpp"
#include <algorithm>
#include <cstddef>
#include <fstream>
#include <initializer_list>
#include <iostream>
#include <map>
#include <memory>
#include <set>
#include <sstream>
#include <string>
#include <string_view>
#include <type_traits>
#include <unordered_map>
#include <unordered_set>
#include <vector>

#include <boost/fusion/adapted/struct.hpp>
#include <boost/fusion/include/for_each.hpp>
#include <boost/type_index.hpp>

#ifndef USE_ROCKSDB
#define USE_ROCKSDB
#include "leveldb/db.h"
namespace kvdb = leveldb;
#else
#include "rocksdb/db.h"
namespace kvdb = rocksdb;
#endif

namespace lava {
kvdb::DB *db;
kvdb::Options options;
kvdb::Status s;
std::string path = "./kv";

void demand(bool cond, std::string str) {
  if (!(cond)) {
    std::cout << str << std::endl;
    exit(1);
  }
}

void put(const std::string key, const std::string value) {
  assert(db->Put(kvdb::WriteOptions(), key, value).ok());
}

void get(const std::string key, std::string &value) {
  s = db->Get(kvdb::ReadOptions(), key, &value);
  demand(s.ok(), s.ToString() + key);
}

bool check_exists(const std::string key) {
  std::string result;
  s = db->Get(kvdb::ReadOptions(), key, &result);
  if (s.IsNotFound()) {
    return false;
  } else {
    demand(s.ok(), s.ToString());
    return true;
  }
}
void put_if_not_exists(const std::string key, const std::string value) {
  if (!check_exists(key))
    put(key, value);
}

template <typename T, typename Tcheck = void> struct Ref;

Ref<size_t> *numids;

template <typename T>
struct Ref<T, typename std::enable_if<std::conjunction<
                  std::disjunction<std::negation<is_begin<T>>,
                                   std::is_same<T, std::string>>,
                  std::negation<is_begin_points_first<T>>>::value>::type> {
  // key in kvstore
  std::string id;
  // cast to T type
  operator T() const {
    std::string result;
    get(id, result);
    T val;
    ::Unpack(result, &val);
    return val;
  }
  Ref<T> operator++(int) {
    T v = *this;
    *this = v + 1;
    return v;
  }
  Ref<T> &operator=(const T &v) {
    put(id, ::Pack(&v));
    return *this;
  }
  Ref<T> &operator=(const Ref<T> &v) {
    // Guard self assignment
    if (this == &v)
      return *this;

    *this = T(v);
    return *this;
  }
  Ref() {
    std::cerr << "initialize literal Ref\n";
    // for creating new key / empty ref
    // set the Ref's id and then increment
    size_t prev_num_ids = *numids;
    // I think that
    // for some reason when we set size_t prev_num_ids = (*numids)++
    // the compiler casts to a size_t and then increments
    (*numids)++;
    id = ::Pack(&prev_num_ids);
    // initialize
    *this = T();
  }
  Ref(const std::string_view id_) { id = id_; }
  Ref(size_t id_) { id = ::Pack(&id_); }
};
bool is_initialized = false;
void init() {
  options.create_if_missing = true;
  s = kvdb::DB::Open(options, path, &db);
  demand(s.ok(), s.ToString());
  numids = new Ref<size_t>("");
  std::string result;
  s = db->Get(kvdb::ReadOptions(), "", &result);
  if (s.IsNotFound()) {
    std::cout << "initializing lava::numids = 0\n";
    *numids = 0;
  } else {
    demand(s.ok(), s.ToString());
    std::cout << "found existing lava::numids = " << *numids << "\n";
  }
  is_initialized = true;
}

// Ref to lavavector
template <template <typename, typename...> typename Collection, typename T,
          typename... Args>
// do not treat Ref as container
struct Ref<Collection<T, Args...>,
           typename std::enable_if<!std::disjunction<
               std::is_same<Collection<T>, Ref<T>>,
               std::is_same<Collection<T>, std::string>,
               // must have .begin() method
               is_not_begin<Collection<T>>,
               is_begin_points_first<Collection<T>>>::value>::type> {
  // key in kvstore
  std::string id;
  Ref<size_t> len;

  // to avoid unnecessary disk access and wasting top-level kv ids, we must
  // include the len member in the constructor intializer list which avoids
  // default construction w/o arguments that would auto-select a key
  Ref() : len{""} {
    // for creating new key / empty ref
    // set id and increment
    demand(lava::is_initialized,
           "must run lava::init() before declaring disk-backed refs");
    size_t prev_num_ids = *numids;
    (*numids)++;
    id = Pack(&prev_num_ids);
    len.id = id;
    len = 0;
  }
  Ref(size_t id_) : len{""} {
    demand(lava::is_initialized,
           "must run lava::init() before declaring disk-backed refs");

    id = Pack(&id_);
    len.id = id;

    // if not exists, initialize the len
    size_t zero = 0;
    put_if_not_exists(len.id, Pack(&zero));
  }
  Ref(std::string id_) : id{id_}, len{id_} {
    // if not exists, initialize the len
    demand(lava::is_initialized,
           "must run lava::init() before declaring disk-backed refs");
    size_t zero = 0;
    put_if_not_exists(len.id, Pack(&zero));
  }

  Ref<T> operator[](const size_t &i) { return Ref<T>(id + Pack(&i)); }
  Ref<T> at(const size_t &i) {
    assert(i < len);
    return (*this)[i];
  }
  operator Collection<T, Args...>() {
    Collection<T, Args...> val;
    for (size_t i = 0; i < len; i++) {
      T elem = this->at(i);
      val.insert(val.end(), elem);
    }
    return val;
  }
  Ref<Collection<T, Args...>> &operator=(const Collection<T, Args...> &v) {
    len = 0;
    for (auto elem : v)
      push_back(elem);
    return *this;
  }
  Ref<Collection<T, Args...>> &operator=(const Ref<Collection<T, Args...>> &v) {
    // Guard self assignment
    if (this == &v)
      return *this;

    *this = Collection<T, Args...>(v);
    return *this;
  }
  size_t size() { return len; }
  void push_back(const T &val) {
    size_t i = len;
    (*this)[i] = val;
    len++;
  }
  void insert(const T &val) { this->push_back(val); }
};

bool startswith(const std::string &s, const std::string &start) {
  auto mm = std::mismatch(start.begin(), start.end(), s.begin(), s.end());
  if (mm.first == start.end()) {
    return true;
  }
  return false;
}
// Ref to lavamap
template <template <typename, typename, typename...> typename Mapping,
          typename T1, typename T2, typename... Args>
struct Ref<Mapping<T1, T2, Args...>,
           typename std::enable_if<
               is_begin_points_first<Mapping<T1, T2, Args...>>::value>::type> {
  // key in kvstore
  std::string id;
  Ref<size_t> len;

  Ref<T2> operator[](const T1 &k) {
    std::string key = id + Pack(&k);
    // increment for new keys
    if (!check_exists(key)) {
      Ref<T2> r(key);
      r = T2();
      len++;
    }
    return Ref<T2>(key);
  }
  int count(const T1 &k) { return check_exists(id + Pack(&k)); }
  Ref<T2> at(const T1 &k) {
    // check this key stores a value (either the T2 value packed or the len for
    // a lavamap/lavavector)
    assert(check_exists(id + Pack(&k)));
    return (*this)[k];
  }
  operator Mapping<T1, T2, Args...>() const {
    Mapping<T1, T2, Args...> val;

    // Get a database iterator
    std::shared_ptr<kvdb::Iterator> db_iter(
        db->NewIterator(kvdb::ReadOptions()));

    // seek to prefix
    db_iter->Seek(id);
    // skip this ref's len
    db_iter->Next();
    std::unordered_set<std::string> seen_ids;
    while (db_iter->Valid()) {
      std::string key_str = db_iter->key().ToString();

      // halt iteration when outside ref id prefix
      if (!startswith(key_str, id)) {
        // we are past the prefix range
        break;
      }
      bool seen = false;
      for (auto s : seen_ids) {
        // could seek as an optimization
        if (startswith(key_str, s)) {
          seen = true;
          break;
        }
      }
      if (seen) {
        // we have already seen this id
        db_iter->Next();
        continue;
      }

      seen_ids.insert(key_str);
      // trim id from key
      std::string map_key_str = key_str.substr(id.length());
      T1 key;
      ::Unpack(map_key_str, &key);
      // set the key-value
      val[key] = Ref<T2>(key_str);
      db_iter->Next();
    }
    return val;
  }
  Ref<Mapping<T1, T2, Args...>> &operator=(const Mapping<T1, T2, Args...> &v) {
    len = 0;
    for (auto it : v)
      (*this)[it.first] = it.second;
    return *this;
  }
  Ref<Mapping<T1, T2, Args...>> &
  operator=(const Ref<Mapping<T1, T2, Args...>> &v) {
    // Guard self assignment
    if (this == &v)
      return *this;

    *this = Mapping<T1, T2, Args...>(v);
    return *this;
  }
  size_t size() { return len; }
  // to avoid unnecessary disk access and wasting top-level kv ids, we must
  // include the len member in the constructor intializer list which avoids
  // default construction w/o arguments that would auto-select a key
  Ref() : len{""} {
    // for creating new key / empty ref
    demand(lava::is_initialized,
           "must run lava::init() before declaring disk-backed refs");
    // set id and increment
    size_t prev_num_ids = *numids;
    (*numids)++;
    id = Pack(&prev_num_ids);
    len.id = id;
    len = 0;
  }
  Ref(size_t id_) : len{""} {
    demand(lava::is_initialized,
           "must run lava::init() before declaring disk-backed refs");
    id = Pack(&id_);
    len.id = id;
    // if not exists, initialize the len
    size_t zero = 0;
    put_if_not_exists(len.id, Pack(&zero));
  }
  Ref(std::string id_) : id{id_}, len{id_} {
    demand(lava::is_initialized,
           "must run lava::init() before declaring disk-backed refs");
    // if not exists, initialize the len
    size_t zero = 0;
    put_if_not_exists(len.id, Pack(&zero));
  }
};

} // namespace lava

#endif
