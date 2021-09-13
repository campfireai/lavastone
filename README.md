# lavastone 🌋🪨
[![MIT License](https://img.shields.io/badge/license-MIT-green.svg)](LICENSE)
[![CI](https://github.com/campfireai/lavastone/actions/workflows/demo.yml/badge.svg)](https://github.com/campfireai/lavastone/actions/workflows/demo.yml)

Lavastone aims to implement standard-library containers transparently backed by disk, using [LevelDB](https://github.com/google/leveldb) as a storage backend.

_Replace this:_
```c++
vector<string> myvec;
myvec.push_back("hello, world!\n");
std::cout << myvec.at(0);
```
_with this:_
```c++
lava::Ref<vector<...> myvec
myvec.push_back("hello, world!\n");
std::cout << myvec.at(0);
```
_How does this work?_
There are several techniques that go into this.
- The `lava::Ref::operator=` is overloaded to instrument stores to a Ref to update the data on disk.
- An implicit conversion operator `lava::Ref<T>::operator T ()`  implements the load from disk to allow Refs to be converted (in most cases*) automatically by the compiler to the referred data type
- [SFINAE](https://en.cppreference.com/w/cpp/language/sfinae) is used to split the implementation of `lava::Ref` for Vector-like and Map-like types

_Why would you use this?_
Imagine you implemnted some C++ code that ran on a big server, and now need to port it over to a mobile phone with reduced memory.
Or alteratively, you used to run it on smaller datasets and the dataset has outgrown your memory space.
My favorite use case, however, is very fast checkpoint-resume. By doing this:
```
lava::Ref<MyComplicatedType> mydata_ondisk(0);
mydata_ondisk= mydata_inmemory;
```
you chose a unique identifier for your datastructure which will persist across runs.
If we start the app again and declare  `lava::Ref<MyComplicatedType> mydata_ondisk(0);` it can start serving requests *immediately* and potentially with acceptable slowdown.

Ideas to extend Lavastone:
- Implement caching, flushing
- Optimize leveldb seek for prefix traversal in lava::Ref<map> --> map conversion
- Allow deletion of keys, resizing etc. for all types
- Implement unordered_sets, sets, and ordered maps properly


TL;DR: Serialize huge container datastructures to a key-value store (LevelDB), allowing fast disk-backed access in production.

Serialize and deserialize C++ variables with Lavastone!
Why would you use our 200-line header file?
- It supports many standard types (`int`, `vector`, `map`, ... out of the box)
- It is lightweight and simple to use
- It is easily [extensible to other data types](#extending-to-other-data-types), without requiring changes to the class or struct definitions

### Motivation
We built Lavastone for an application that needed to handle user queries to a big dataset. These queries could not be efficiently executed by an off-the-shelf SQL or graph-based database. Instead, our code populated many large datastructures that could be used to efficiently respond to the queries in production.

The problem we had was that building these index datastructures could take several hours or days, which caused an unacceptable "boot time" of the application. To reduce the boot time, we built these datastructures once in an offline stage, serialize them to disk using Lavastone, and then loaded them back into memory in a matter of seconds to run in production.

1. __Build stage:__ (slow, takes several hours) Parse, clean, aggregate, and index the data with large datastructures and store these to disk
2. __Production stage:__ (fast) Quickly load the index datastructures from disk and respond to user queries about the data using our custom application logic

Lavastone is highly extensible: with one line it can accommodate arbitrary struct types via the fabulous [Boost Fusion library](https://www.boost.org/doc/libs/1_76_0/libs/fusion/doc/html/index.html).
It can similarly accommodate most custom container types that support iteration, see [Usage](#usage).

Please see the docs below to [get started](#compile-and-run-tests).

## Getting Started
### Compile and run tests
```bash
git clone https://github.com/campfireai/lavastone.git
cd lavastone
mkdir build
cd build
cmake ..
cmake --build . --parallel
./test_lavastone 10000
```

### Adapting a custom struct type
```c++
#include <string>
#include "lavastone.hpp"


struct MyStruct {
    int my_int;
    std::string my_string;
};

LAVASTONE_ADAPT_STRUCT(MyStruct, my_int, my_string);

```
Now lavastone knows how to serialize and deserialize your struct.
```c++
// serialize
MyStruct foo = {.my_int=42, .my_string="hello world"};
pack_to_file(&foo, "foo.struct");

// deserialize
MyStruct foo_new;
unpack_from_file("foo.struct", &foo_new);
```

### Usage
The `pack_to_file` and `unpack_from_file` convenience functions simply wrap the core serialization engine, which consists of a library of `Pack` and `Unpack` functions implemented for various fixed-width and container types.
Use these methods to access the raw strings that Lavastone serializes to / from.

#### Extending to other data types
If you need to extend to other container types, see the `PACK_1D_CONTAINER` and `PACK_2D_CONTAINER` macros in `lavastone.hpp`. Most likely, if you want to extend to a map-like type `MyMap`, you can have Lavastone declare the appropriate templates by adding two lines:
```c++
PACK_2D_CONTAINER(MyMap);
UNPACK_2D_CONTAINER(MyMap);
```
Similarly for a 1D iterable container e.g. `MyVector`, you could use
```c++
PACK_1D_CONTAINER(MyVector);
UNPACK_1D_CONTAINER(MyVector);
```
and then `Pack`, `pack_to_file`, and the associated deserializers should "just work".


## Development and Contributing
Interested in contributing? We 💙 pull requests!
If you have any suggestions or find a bug, you can [open an issue](https://github.com/deepgram-devs/meeting-bot/issues/new) on this repository.
Please be nice 🙂
