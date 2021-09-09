#ifndef __LAVASTONE_HPP__
#define __LAVASTONE_HPP__

#include <fstream>
#include <iostream>
#include <type_traits>
#include <memory>
#include <sstream>
#include <string_view>
#include <set>
#include <unordered_set>
#include <map>
#include <unordered_map>

#include <boost/fusion/adapted/struct.hpp>
#include <boost/fusion/include/for_each.hpp>
#include <boost/type_index.hpp>


#include "leveldb/db.h"
namespace kvdb=leveldb;


static std::string Pack(const std::string *data) { return *data; }

template <typename T>
static std::unique_ptr<T> Unpack(const std::string_view &data) {
  if (data.size() != sizeof(T))
    return nullptr;

  auto d = std::make_unique<T>();
  memcpy(d.get(), data.data(), data.size());
  return d;
}

template <typename T> static void Unpack(const std::string_view &data, T *out) {

  if (data.size() != sizeof(T)) {
    auto demangled_name = boost::typeindex::type_id<T>().pretty_name();
    std::cerr << "invalid string size for type " << demangled_name << std::endl;
    std::cerr << "received data with size " << data.size() << "\n";
    std::cerr << "sizeof(" << demangled_name << ") = " << sizeof(T) << "\n";
    std::cerr << "data = " << data << "\n";
    exit(1);
  }

  auto d = std::make_unique<T>();
  memcpy(d.get(), data.data(), data.size());
  *out = *d;
}

static std::unique_ptr<std::string> Unpack(const std::string &data) {
  auto d = std::make_unique<std::string>();
  *d = data;
  return d;
}

static void Unpack(const std::string_view &data, std::string *out) {
  *out = std::string(data);
}

template <typename T>
static void Unpack(const std::string_view &data, std::vector<T> *vp) {
  size_t i = 0;
  size_t j = 0;
  // std::cerr<<"unpacking vector"<<std::endl;
  while (i < data.size()) {
    size_t vslen;
    Unpack(data.substr(i, sizeof(vslen)), &vslen);
    i += sizeof(vslen);
    T v;
    Unpack(data.substr(i, vslen), &v);
    i += vslen;
    if (vp->size() < j + 1)
      vp->resize(j + 1);
    (*vp)[j] = v;
    j++;
  }
}

// unpack via vector
template <template <typename, typename...> typename Collection, typename T, typename... Args>
static void Unpack(const std::string_view &data, Collection<T, Args...> *vp) {
  std::vector<T> vdata;
  Unpack(data, &vdata);
  for (auto elem : vdata)
    vp->insert(elem);
}


// detect map-like objects
template <typename T, typename = void>
struct is_begin_points_first : std::false_type {};
template <typename T>
struct is_begin_points_first<T, std::void_t<decltype(std::declval<T>().begin()->first)>> : std::true_type {};


// // detect incrementable objects
// template <typename T, typename = void>
// struct has_operator : std::false_type {};
// template <typename T>
// struct has_operator<T, std::void_t<decltype(std::declval<T>().begin()->first)>> : std::true_type {};


// unpack map-like objects
template <template <typename, typename, typename...> typename Mapping, typename T1, typename T2, typename... Args>
static void Unpack(const std::string_view &data, Mapping<T1, T2> *mp, typename std::enable_if<is_begin_points_first<Mapping<T1, T2, Args...>>::value>::type* dummy = 0) {
  size_t i = 0;
  while (i < data.size()) {
    size_t kslen, vslen;
    Unpack(data.substr(i, sizeof(kslen)), &kslen);
    i += sizeof(kslen);
    T1 k;
    Unpack(data.substr(i, kslen), &k);
    i += kslen;
    Unpack(data.substr(i, sizeof(vslen)), &vslen);
    i += sizeof(vslen);
    T2 v;
    Unpack(data.substr(i, vslen), &v);
    i += vslen;
    (*mp)[k] = v;
  }
}


template <typename T> static std::string PackData(const T *data) {
  std::string d(sizeof(T), L'\0');
  memcpy(&d[0], data, d.size());
  return d;
}

static inline std::string Pack(const int *data) { return PackData(data); }

static inline std::string Pack(const unsigned *data) { return PackData(data); }

static inline std::string Pack(const size_t *data) { return PackData(data); }

template <typename T> std::string Pack(std::vector<T> *data) {
  std::string packed_string = "";
  for (auto v : *data) {
    auto vs = Pack(&v);
    size_t vslen = vs.size();
    packed_string += Pack(&vslen) + Pack(&vs);
  }
  return packed_string;
}


// pack via vector
template <template <typename, typename...> typename Collection, typename T,
          typename... Args>
std::string Pack(const Collection<T, Args...> *data) {
  std::vector<T> vdata;
  for (auto elem : *data)
    vdata.push_back(elem);
  return Pack(&vdata);
}


// pack map-like objects
template <template <typename, typename, typename...> typename Mapping, typename T1, typename T2, typename... Args>
std::string Pack(const Mapping<T1, T2, Args...> *data, typename std::enable_if<is_begin_points_first<Mapping<T1, T2, Args...>>::value>::type* dummy = 0) {
  std::string packed_string = "";
  for (auto kv : *data) {
    auto ks = Pack(&kv.first);
    size_t kslen = ks.size();
    auto vs = Pack(&kv.second);
    size_t vslen = vs.size();
    packed_string += Pack(&kslen) + Pack(&ks) + Pack(&vslen) + Pack(&vs);
  }
  return packed_string;
}


template <typename T> std::string FusionStructPack(const T *data) {
  std::string packed_string;
  boost::fusion::for_each(*data, [&](auto arg1) {
    auto packed = Pack(&arg1);
    unsigned long len = packed.size();
    packed_string += Pack(&len) + packed;
  });
  return packed_string;
}

template <typename T>
std::string FusionStructUnpack(const std::string_view &data, T *out) {
  std::string packed_string;
  int i = 0;
  boost::fusion::for_each(*out, [&](auto &arg1) {
    unsigned long len;
    Unpack(data.substr(i, sizeof(unsigned long)), &len);
    i += sizeof(unsigned long);
    Unpack(data.substr(i, len), &arg1);
    i += len;
  });
  return packed_string;
}

#define LAVASTONE_ADAPT_STRUCT(T, ...)                                         \
  BOOST_FUSION_ADAPT_STRUCT(T, __VA_ARGS__);                                   \
  std::string Pack(const T *d) { return FusionStructPack(d); }                 \
  void Unpack(const std::string_view &data, T *d) {                            \
    FusionStructUnpack(data, d);                                               \
  }

template <typename T> void pack_to_file(T *obj, std::string fname) {
  std::ofstream out(fname);
  out << Pack(obj);
  out.close();
}

template <typename T> void unpack_from_file(std::string fname, T *obj) {
  std::ifstream in(fname);
  std::string packed;
  if (in) {
    std::ostringstream ss;
    ss << in.rdbuf();
    packed = ss.str();
  }
  in.close();
  Unpack(std::string_view(packed), obj);
}


namespace lava {
  kvdb::DB *db;
  kvdb::Options options;
  kvdb::Status s;
  std::string path = "./kv";

  std::atomic<size_t> atomic_lavaref_count{0};

  void demand(bool cond, std::string str) {
    if (!(cond)) {
      std::cout << str << std::endl;
      exit(1);
    }
  }

  void put (const std::string key, const std::string value) {
    assert(db->Put(kvdb::WriteOptions(), key, value).ok());
  }

  void get(const std::string key, std::string& value) {
    s = db->Get(kvdb::ReadOptions(), key, &value);
    demand(s.ok(), s.ToString());
  }

  template <typename T>
  struct Ref;

  Ref<size_t>* numids;


  template <typename T>
  struct Ref {
    // key in kvstore
    std::string id;
    // cast to T type
    operator T () const {
      std::string result;
      get(id, result);
      T val;
      ::Unpack(result, &val);
      return val;
    }
    Ref<T>& operator= (const Ref<T>& v) {
      // Guard self assignment
      if (this == &v)
        return *this;

      // demand(false, "assign to literal ref");
      id = v.id;
      return *this;
    }
    Ref<T> operator++ (int) {
      std::cerr << "here0\n";
      T v = *this;
      std::cerr << "here1\n";
      *this = v+1;
      std::cerr << "here2\n";
      return v;
    }
    Ref<T>& operator= (const T& v) {
      put(id, ::Pack(&v));
      return *this;
    }
    Ref() {
      std::cerr << "initialize literal Ref\n";
      // for creating new key / empty ref
      // set the Ref's id and then increment
      std::cerr << "numids = " << numids << "\n";
      std::cerr << "numids->id = " << numids->id << "\n";
      std::cerr << "*numids = " << *numids << "\n";
      std::cerr << "gonna get prev_num_ids\n";
      size_t prev_num_ids = *numids;
      std::cerr << "prev_num_ids = " << prev_num_ids << "\n";
      // I think that
      // for some reason when we set size_t prev_num_ids = (*numids)++
      // the compiler casts to a size_t and then increments
      (*numids)++;
      id = ::Pack(&prev_num_ids);
      std::cerr << "here0.0\n";
      // initialize
      *this = T();
      std::cerr << "here0.1\n";
    }
    Ref(std::string id_) {
      std::cout << "initialize literal ref with std::string id\n";
      id = id_;
    }
    Ref(size_t id_) {
      id = ::Pack(&id_);
    }
  };

  void init() {
    options.create_if_missing = true;
    s = kvdb::DB::Open(options, path, &db);
    demand(s.ok(), s.ToString());

    std::cerr << "numids = " << numids << "\n";
    numids = new Ref<size_t>("");
    std::cerr << "numids = " << numids << "\n";
    std::string result;
    s = db->Get(kvdb::ReadOptions(), "", &result);
    if (s.IsNotFound()) {
      std::cout << "initializing numids = 0\n";
      *numids = 0;
      std::cerr << "*numids = " << *numids << "\n";
      std::cerr << "numids->id = " << numids->id << "\n";
    } else {
      demand(s.ok(), s.ToString());
      std::cout << "found existing numids = " << *numids << "\n";
    }
  }

  // Ref to lavavector
  template <template <typename, typename...> typename Collection, typename T, typename... Args>
  struct Ref<Collection<T, Args...>> {
    // key in kvstore
    std::string id;
    Ref<size_t> len;

    Ref<T> at(size_t i) {
      assert(i<len);
      std::string result;
      return Ref<T>(id + Pack(&i));
    }
    // cast to Collection<T, Args...> type
    operator Collection<T, Args...> () const {
      Collection<T, Args...> val;
      demand(false, "unimplemented cast to collection");
      for (size_t i=0; i<len; i++) {
        val.insert(this->at(i));
      }
      return val;
    }
    Ref<Collection<T, Args...>>& operator= (const Ref<Collection<T, Args...>>& v) {
      // Guard self assignment
      if (this == &v)
          return *this;
      // std::cerr << "v = " << v << "\n";
      demand(false, "assigned to another ref in collection");
    }

    Ref<Collection<T, Args...>>& operator= (const Collection<T, Args...>& v) {
      demand(false, "unimplemented set equals to collection");
      return *this;
    }
    size_t size() {
      return len;
    }
    void push_back (const T& val) {
      std::cout << "len.id = " << len.id << "\n";
      std::cout << "id = " << id << "\n";
      size_t i = len;
      put(id + Pack(&i), Pack(&val));
      len++;
    }
    void insert (const T& val) {
      this->push_back(val);
    }
    Ref() {
      std::cerr << "initialize container Ref\n";
      // for creating new key / empty ref
      // set id and increment
      size_t prev_num_ids = *numids;
      (*numids)++;
      std::cerr << "prev_num_ids = " << prev_num_ids << "\n";
      std::cerr << "herea.0\n";
      id = Pack(&prev_num_ids);
      std::cerr << "herea.1\n";
      std::cout << "len.id = " << len.id << "\n";
      size_t id_size_t;
      Unpack(id, &id_size_t);
      std::cout << "len.id = " << len.id << "\n";
      std::cout << "id = " << id << "\n";
      std::cout << "id_size_t = " << id_size_t << "\n";
      len = Ref<size_t>(id);
      std::cout << "len.id = " << len.id << "\n";
      std::cout << "id = " << id << "\n";
      std::cout << "id_size_t = " << id_size_t << "\n";
      std::cerr << "herea.2\n";
      len = 0;
      std::cerr << "herea.3\n";
    }
    Ref<Collection<T, Args...>>(size_t id_) {
      std::cerr << "initialize container with size_t id\n";
      id = Pack(&id_);
      len = Ref<size_t>(id_);
    }
    Ref<Collection<T, Args...>>(std::string id_) {
      std::cerr << "initialize container with std::string id\n";
      id = id_;
      std::cerr << "declaring len ref with std::string id\n";
      len = Ref<size_t>(id);
      std::cerr << "finished initialize container with std::string id\n";
    }
  };

  template<typename T>
  std::string Pack(const Ref<T> *data) {
    return data->id;
  }

  template<typename T>
  void Unpack(const std::string_view &data, Ref<T> *r) {
    *r = Ref<T>(data);
  }


  //
  // // unpack map-like objects
  // template <template <typename, typename, typename...> typename Mapping, typename T1, typename T2, typename... Args>
  // static void Unpack(const std::string_view &data, Mapping<T1, T2> *mp, typename std::enable_if<is_begin_points_first<Mapping<T1, T2, Args...>>::value>::type* dummy = 0) {
  // Ref<T2> ContainerRef<Mapping<T1, T2, Args...>>::operator[] () {
  //
  // }
  // template<typename T>
  // Ref::operator += (const T& v, std::enable_if<>::type* dummy = 0) {
  //
  // }
}

#endif
