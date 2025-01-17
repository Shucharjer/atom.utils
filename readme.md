# atom.utils

[简体中文](readme_zh.md) | English

atom.utils is a collection of basic tools used in the atom engine. It is a headers-only modern C++ library that is easy to link in various projects. Now it includes some data structures (such as sparse_map, compressed_pair, spin_lock, etc.) as well as reflection and its corresponding serialization.
The purpose of this library is to provide easy-to-use modern C++ basic tools to help users quickly build content in modern C++.

## Intro

### Ranges

As we all know, std::ranges::to provided in C++23 is a very useful function that can build a container of another type from a range of one type with simple syntax. 
Now, it has been brought to C++20. If you use C++23, the implementation in the standard library will be called.

```c++

```

### Reflection

```c++
struct a {
    int member1;
    char member2;
};

BEGIN_TYPE(a)
FIELDS(FIELD(member1))
END_TYPE(a, a_register)

TEST_CASE("reflection") {
    SECTION("basic usage") {
        
    }
}

```