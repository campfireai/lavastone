#ifndef __LAVAPACK_HPP__
#define __LAVAPACK_HPP__

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
#include <type_traits>

#include <boost/fusion/adapted/struct.hpp>
#include <boost/fusion/include/for_each.hpp>
#include <boost/type_index.hpp>


static std::string Pack(const std::string *data) { return *data; }
static std::string Pack(const char *data[]) { return std::string(*data); }
static std::string Pack(const char *data) { return std::string(data); }

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


// detect container objects
template <typename T, typename = void>
struct is_begin : std::false_type {};
template <typename T>
struct is_begin <T, std::void_t<decltype(std::declval<T>().begin())>> : std::true_type {};


// detect push_back method
template <typename T, typename = void>
struct is_push_back : std::false_type {};
template <typename T>
struct is_push_back <T, std::void_t<decltype(std::declval<T>().push_back(T()))>> : std::true_type {};


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


// catch-all declaration
template<typename T>
std::string Pack(const T *data);
template<typename T>
void Unpack(const std::string_view &data, T*mp);

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
#endif
