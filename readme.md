# atom.utils

[简体中文](readme_zh.md) | English

atom.utils is a collection of basic tools used in the atom engine.

It is a headers-only modern C++ library that is easy to link in various projects.

Now it includes some data structures (such as sparse_map, compressed_pair, spin_lock, etc.) as well as reflection and its corresponding serialization.
The purpose of this library is to provide easy-to-use modern C++ basic tools to help users quickly build content in modern C++.

## Quick start

### Compiler requirements

This library only supports C++20 or higher, so, make sure your compiler supports C++20.

- g++ 10 above;
- clang++13 above;
- msvc 14.29 above.

### Install & Compile

#### Manually Install

1. Clone this repo

   ```shell
   git clone https://github.com/Shucahrjer/atom.utils
   cd atom.utils
   ```
2. build

   ```shell
   cmake -B build
   # or you could add other args, such as cmake -DCMAKE_C_COMPILER="clang" -DCMAKE_CXX_COMPILER="clang++" -DCMAKE_MAKE_PROGRAM="ninja" -G "Ninja" -B build
   cd build
   cmake --build . --config debug
   ```
3. install

   ```shell
   cmake --install . # --prefix
   ```
4. start dev

## Intro

### Table of Contents

<pre>
<a href="#Core">Core</a>
    <a href="#Pair">Pair</a>
    <a href="#Pipeline-and-closure">Pipeline and Closure</a>
        <a href="#Adaptor-closure">Adaptor Closure</a>
        <a href="#Closure">Closure</a>
<a href="#Range">Range</a>
<a href="#Reflection">Reflection</a>
    <a href="#No-Macro-Reflection">No Macro Reflection</a>
    <a href="#Custom-Reflection">Custom Reflection</a>
    <a href="#Support-for-Serialization">Support for Serialization</a>
<a href="#Structures">Structures</a>
    <a href="#dense_map">dense_map</a>
    <a href="#dense_set">dense_set</a>
<a href="#Thread">Thread</a>
    <a href="#Thread-Pool">Thread Pool</a>
    <a href="#Coroutine">Coroutine</a>
    <a href="#Spin-Lock">Spin Lock</a>
    <a href="#Hybrid-Lock">Hybrid Lock</a>
<a href="#Signal">Signal</a>
    <a href="#Lambda">Lambda</a>
    <a href="#Delegate">Delegate</a>
    <a href="#Sink">Sink</a>
    <a href="#Dispatcher">Dispatcher</a>
<a href="#Memory">Memory</a>
    <a href="#Memory-Pool">Memory Pool</a>
    <a href="#Allocator">Allocator</a>
    <a href="#Destroyer">Destroyer</a>
    <a href="#Storage">Storage</a>
    <a href="#around_ptr">around_ptr</a>
</pre>

The following content is based on such a statement

```c++
using namespace atom::utils;
```

### Core

#### `pipeline_base`&`pipline_result`&`closure`

Since C++20 does not have the range adapter closure in C++23, the closure type has to be introduced and supported in C++20
As the name implies, `pipeline_base` is the base class that scope closures need to inherit, `pipline_result` is the result of pipeline operations, and `closure` is the closure

```c++
struct get_vector_fn {
template <typename... Args>
std::vector<int> operator()(Args&&... args) {
return std::vector<int>(std::forward<Args>(args)...);
}
};

auto closure = make_closure<get_vector_fn>();
// A vector of 10 elements
auto vector = closure(10);
```

You can use `pipeline_base` to quickly build a scope closure. For example:

```c++
struct empty_view : public ::atom::utils::pipeline_base<empty_view> {
    template <std::ranges::range Rng>
    requires std::is_default_constructible_v<Rng>
    constexpr auto operator()(Rng&& range) const {
        return Rng{};
    }
};

constexpr inline empty_view empty;

...

// do pipeline operation between a range and a range closure
std::vector vector = { 2, 3, 4, 6 };
auto empty_vector  = vector | empty;
assert(empty_vector.empty());

// do pipeline operator between two range closures
auto closure              = empty | std::views::reverse;
auto another_empty_vector = vector | closure;
assert(another_empty_vector.empty());
```

### Range

As we all know, std::ranges::to provided in C++23 is a very useful function that can build a container of another type from a range of one type with simple syntax.
Now, it has been brought to C++20. If you use C++23, the implementation in the standard library will be called.

```c++
auto map = std::map<int, int>{{1, 2}, {2, 1}, {465, 0}, {53, 634}};
auto values = map | std::views::values;
auto vector = ranges::to<std::vector>(values);
```

It is almost the implementation in the standard library, except that C++20 does not have std::from_range in C++23.
It is not difficult to guess that it can also get a closure like in C++23.

```c++
auto closure = ranges::to<std::vector<int>>(4);
auto vector = closure(values);
```

In the process of implementing std::ranges::to, some types are also introduced,
such as `phony_input_iterator`, `pipline_result` and `range_closure`,
which are respectively "fake input iterators" for delayed loading, the results of pipeline operations and closures for delayed loading.
You can try to use them to implement some delayed loading outside the standard library.

### Reflection

```c++
// definition
struct a {
int member1;
char member2;
};

BEGIN_TYPE(a)
FIELDS(FIELD(member1))
END_TYPE()

// usage
auto a = ::a{};
auto reflected = utils::reflected<::a>{};
auto& tuple = reflected.fields();
auto& traits1 = std::get<index_of<"member1">(tuple)>(tuple);
int value1 = traits1.get(a);

auto& traits2 = std::get<index_of<"member2">(tuple)>(tuple);
char value2 = traits1.get(a);
```

Also supports serialization and deserialization of `nlohmann-json`

```c++
// Serialization
nlohmann::json json = a;
std::cout << json.dump(4) << '\n';

// Change the value of member1, expect it to change back to the previous value after deserialization
a.member1 = 90;

// Deserialization
a = json;
std::cout << a.member1 << '\n';
```

### Memory

Provides allocators, memory pools, and storage

#### Memory pool

Provides two types of memory pools

```c++
synchronized_pool synchronized_pool{};
unsynchronized_pool unsynchronized_pool{};
```

#### Allocator

Provides two types of allocators: `standard_allocator` and `allocator`
Among them, `standard_allocator` just encapsulates `std::allocator`
And `allocator` needs to be created from the memory pool

```c++
synchronized_pool pool;
allocator<int, decltype(pool)> allocator{pool};

std::vector<int, decltype(allocator)> vector{allocator};
```

#### Memory

Exclusive memory and shared memory are provided.
Shared memory supports copy-on-write, which is suitable for scenarios with high copy overhead.
It should be noted that this copy-on-write is only effective when `=` is used.

### Structure

#### `dense_map`&`dense_set`

### Signal

#### Delegate

#### Sink

#### Dispatcher
