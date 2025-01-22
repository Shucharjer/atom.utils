# atom.utils

[简体中文](readme_zh.md) | English

atom.utils is a collection of basic tools used in the atom engine. It is a headers-only modern C++ library that is easy to link in various projects. Now it includes some data structures (such as sparse_map, compressed_pair, spin_lock, etc.) as well as reflection and its corresponding serialization.  
The purpose of this library is to provide easy-to-use modern C++ basic tools to help users quickly build content in modern C++.  

## Intro

The following content is based on such a statement

```c++
using namespace atom::utils;
```

### Core

#### pair

`core/pair.hpp` provides `compressed_pair`, `reversed_compressed_pair` and `reversed_pair`,  
Among them, `compressed_pair` is a structure that uses the compiler's empty base class optimization to save memory usage,  
The other two are the reversals of `compressed_pair` and `std::pair`  

```c++
struct empty {};
// Declaration
compressed_pair<int, empty> compressed_pair;
std::pair<int, empty> pair;

// The difference is that first and second are functions
compressed_pair.first() = 114514;
pair.first = 114514;

// 4
std::cout << sizeof(compressed_pair);
// 5
std::cout << sizeof(pair);
```

Reversal can be achieved through the `reverse` function  

```c++
auto& reversed_compressed = reverse(compressed_pair);
// Now second() is the int type
reversed_compressed.second() = 1919810;
// Similarly
auto& reversed = reverse(pair);
reversed.second = 1919810;
```

#### `pipline_result`&`closure`

Since C++20 does not have the range adapter closure in C++23, the closure type has to be introduced and supported in C++20  
As the name implies, `pipline_result` is the result of the pipeline operation, and `closure` is the closure  

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

### Range

As we all know, std::ranges::to provided in C++23 is a very useful function that can build a container of another type from a range of one type with simple syntax.  
Now, it has been brought to C++20. If you use C++23, the implementation in the standard library will be called.  

```c++
auto map = std::map<int, int>{{1, 2}, {2, 1}, {465, 0}, {53, 634}};
auto values ​​= map | std::views;
auto vector = ranges::to<std::vector>(values);
```

It is almost the implementation in the standard library, except that C++20 does not have std::from_range in C++23.  
It is not difficult to guess that it can also get a closure like in C++23.  

```c++
auto closure = ranges::to<std::vector<int>>(4);
auto vector = closure(values);
```

The only difference from the standard library is that when passing in the parameter template as a template parameter, you need to call `to_closure`.

```c++
auto closure = ranges::to_closure<std::vector>(4);
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