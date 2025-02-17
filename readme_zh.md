# atom.utils

[English](readme.md) | 简体中文

atom.utils是atom引擎中所使用到的一系列基础工具的集合，它是一个headers-only的现代C++库，以便于在各种项目中链接。现在它包括一些数据结构(比如sparse_map、compressed_pair、spin_lock等)以及反射和它对应的序列化。
这个库的宗旨是提供易用的现代C++基础工具，帮助用户快速在现代C++中构建内容。

## 快速开始

### 编译器要求

这个库只支持C++20或者更高版本，所以确保你的编译器支持C++20.

- g++ 10 above;
- clang++13 above;
- msvc 14.29 above.

### 安装与编译

#### 手动安装

1. 克隆这个仓库

    ```shell
    git clone https://github.com/Shucahrjer/atom.utils
    cd atom.utils
    ```

2. 构建

    ```shell
    cmake -B build
    # or you could add other args, such as cmake -DCMAKE_C_COMPILER="clang" -DCMAKE_CXX_COMPILER="clang++" -DCMAKE_MAKE_PROGRAM="ninja" -G "Ninja" -B build
    cd build
    cmake --build . --config debug
    ```

3. 安装

```shell
cmake --install . # --prefix
```

4. 开始开发

## 简介

接下来的内容是基于这样一条语句的

```c++
using namespace atom::utils;
```

### 核心

#### `pipeline_base`&`pipline_result`&`closure`

由于C++20并没有C++23中的范围适配器闭包，因此不得不在C++20引入一系列类型并对其进行支持
顾名思义，`pipeline_base`是范围闭包需要继承的基类，`pipline_result`是管道运算后的结果，`closure`就是闭包

```c++
struct get_vector_fn {
    template <typename... Args>
    std::vector<int> operator()(Args&&... args) {
        return std::vector<int>(std::forward<Args>(args)...);
    }
};

auto closure = make_closure<get_vector_fn>();
// 包含10个元素的vector
auto vector = closure(10);
```

你可以通过 `pipeline_base`快速构建一个范围闭包，可以参考以下示例

```c++
struct empty_fn : public ::atom::utils::pipeline_base<empty_view> {
    template <std::ranges::range Rng>
    requires std::is_default_constructible_v<Rng>
    constexpr auto operator()(Rng&& range) const {
        return Rng{};
    }
};

constexpr inline empty_fn empty;

...

// 范围与构建的范围闭包进行管道操作
std::vector vector = { 2, 3, 4, 6 };
auto empty_vector  = vector | empty;
assert(empty_vector.empty());

// 范围闭包与其它闭包类型进行管道操作
auto closure              = empty | std::views::reverse;
// 范围与上述产生的结果进行管道操作
auto another_empty_vector = vector | closure;
assert(another_empty_vector.empty());
```

### 范围

众所周知，C++23中所提供的std::ranges::to是一个非常好用的函数，它能用简单的语法从一种类型的范围构建另一种类型的容器。
现在，它被带到了C++20，如果使用C++23，那么会调用标准库中的实现。

```c++
auto map = std::map<int, int>{{1, 2}, {2, 1}, {465, 0}, {53, 634}};
auto values = map | std::views::values;
auto vector = ranges::to<std::vector>(values);
```

它几乎是标准库中的实现，除了C++20没有C++23中的std::from_range。
不难推测，它也能像C++23中一样获得一个闭包。

```c++
auto closure = ranges::to<std::vector<int>>(4);
auto vector = closure(values);
```

在实现std::ranges::to的过程中，也引入了一些类型，
比如 `phony_input_iterator`、`pipline_result`和 `range_closure`，
它们分别是用于延迟加载的“假输入迭代器”，管道运算的结果和用于延迟加载的闭包。
可以尝试使用它们来实现一些标准库之外的延迟加载。

### 反射

```c++
// definition
struct a {
    int member1;
    char member2;
};

// usage
auto a = ::a{};
auto& member1 = utils::get<0>(a);
```

也支持了 `nlohmann-json`的序列化与反序列化

```c++
// 序列化
nlohmann::json json = a;
std::cout << json.dump(4) << '\n';

// 更改member1的值，期待它在反序列化之后变回之前的值
a.member1 = 90;

// 反序列化
a = json;
std::cout << a.member1 << '\n';
```

### 内存

提供了分配器、内存池和存储器

#### 内存池

提供了两种类型的内存池

```c++
synchronized_pool synchronized_pool{};
unsynchronized_pool unsynchronized_pool{};
```

#### 分配器

提供了两种类型的分配器：`standard_allocator`和 `allocator`
其中，`standard_allocator`只是将 `std::allocator`封装了一下
而 `allocator`则需要从内存池创建

```c++
synchronized_pool pool;
allocator<int, decltype(pool)> allocator{pool};

std::vector<int, decltype(allocator)> vector{allocator};
```

#### 存储器

提供了独占式的存储器和共享式的存储器。
共享式存储器支持写时复制，适合用在复制开销较大的场景
需要注意的是，这种写时复制只在使用 `=`时生效

### 结构

#### `dense_map`&`dense_set`

### 信号

#### 委托

#### 槽

#### 调度器
