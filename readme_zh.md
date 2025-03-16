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

### 目录

<pre>
<a href="#核心">核心</a>
    <a href="#对">对</a>
    <a href="#管道与闭包">管道与闭包</a>
        <a href="#适配器闭包">适配器闭包</a>
        <a href="#闭包">闭包</a>
<a href="#范围">范围</a>
<a href="#反射">反射</a>
    <a href="#无宏反射">无宏反射</a>
    <a href="#自定义反射">自定义反射</a>
    <a href="#序列化支持">序列化支持</a>
<a href="#结构">结构</a>
    <a href="#dense_map">dense_map</a>
    <a href="#dense_set">dense_set</a>
<a href="#线程">线程</a>
    <a href="#线程池">线程池</a>
    <a href="#协程">协程</a>
    <a href="#自旋锁">自旋锁</a>
    <a href="#混合锁">混合锁</a>
<a href="#信号">信号</a>
    <a href="#lambda">lambda</a>
    <a href="#委托">委托</a>
    <a href="#槽">槽</a>
    <a href="#调度器">调度器</a>
<a href="#内存">内存</a>
    <a href="#内存池">内存池</a>
    <a href="#分配器">分配器</a>
    <a href="#销毁器">销毁器</a>
    <a href="#存储">存储</a>
    <a href="#around_ptr">around_ptr</a>
</pre>

### 核心

#### 对

提供了压缩对、逆转的压缩对、逆转的对，并且它们都支持结构化绑定

```c++
struct empty {};

compressed_pair<empty, int> cpair;
empty& e = cpair.first();
auto& rcpair reverse(cpair);
e = rcpair.second();
const auto& [f, s] = cpair;
const auto& [rf, rs] = rcpair;

std::pair<int, char> pair;
auto& rpair = reverse(pair);
```

#### 管道与闭包

##### 适配器闭包

由于C++20并没有C++23中的范围适配器闭包，因此不得不在C++20引入一系列类型并对其进行支持
顾名思义，`pipeline_base`是范围闭包需要继承的基类，`pipline_result`是管道运算后的结果，`closure`就是闭包

你可以通过 `pipeline_base`快速构建一个范围闭包，可以参考以下示例

```c++
struct empty_fn {
    using pipeline_tag = pipeline_tag;

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

需要指出的是，它不仅仅是范围适配器闭包，它是真的适配器闭包。
这意味着你可以将对其它类型进行管道操作符，只要提供了合适的 `operator()`。

##### 闭包

除了范围适配器闭包外，还提供了一种特殊的闭包，通过它，我们能快速实现惰性构造

```c++
struct get_vector_fn {
    template <typename... Args>
    std::vector<int> operator()(Args&&... args) {
        return std::vector<int>(std::forward<Args>(args)...);
    }
};

std::vector origin = { 2, 2, 3, 34, 2, 523, 53, 5, 346, 54, 645, 7, 4567, 56, 75 };
auto end = origin.cend();
auto closure       = make_closure<get_vector_fn>(end);

auto vec = closure(origin.cbegin() + 2);
auto vec2 = (origin.cbegin() + 2) | closure;
// 3, 34, 2, 523, 53, 5, 346, 54, 645, 7, 4567, 56, 75
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
#include <reflection.hpp> // 仅需包含单个头文件
```

#### 无宏反射

这一部分仅支持聚合类型。

```c++
// definition
struct a {
    int m1;
    char m2;
};
```

##### 类型名

```c++
auto name = name_of<a>();
```

##### 成员数量

```c++
auto count = member_count_v<a>();

// consteval
// auto count = member_count_of<a>();
```

##### 成员名

```c++
auto names = member_names_of<a>();
```

##### 获取成员

```c++
a inst {};
auto& m1 = get<0>(inst);
auto& m2 = get<"m2">(inst);
```

##### 成员偏移（成员指针）

```c++
a inst {};
auto& offsets = offsets_of<a>();
inst.*(std::get<0>(offsets)) = 114514;
inst.*(std::get<1>(offsets)) = 'b';
```

#### 自定义反射

```c++
class b {
    int m1;
    std::vector<char> m2;

    void print() { std::ranges::for_each(m2, [](const auto v) { std::cout << v << ' '; }); }
public:
    REFL_MEMBERS(b, m1, m2)
    REFL_MEMBERS(b, print)
};
```

其余用法与之前一样。

在使用MSVC时，尽管宏所展开的内容是正常的，但可能需要将宏内联展开！

#### 序列化支持

也支持了 `nlohmann-json`的序列化与反序列化

```c++
// 序列化
nlohmann::json json = inst;
std::cout << json.dump(4) << '\n';

// 更改member1的值，期待它在反序列化之后变回之前的值
inst.m1 = 90;

// 反序列化
inst = json;
std::cout << inst.m1 << '\n';
```

### 线程

#### 线程池

#### 协程

#### 自旋锁

#### 混合锁

### 结构

#### dense_map

#### dense_set

### 信号

#### lambda

带有捕获列表的lambda表达式无法隐式转换为指针。
现在，我们能通过一个函数快速获取到指针。

```c++
auto lambda = [&](){};
auto ptr = make_function_ptr<lambda>(); 
// void(*ptr)() = make_function_ptr<lambda>();
```

#### 委托

```c++
delegate<void()> d;
d.bind<lambda>();
// 或者一步到位
delegate<void()> d2 { spread_arg<lambda> };
```

#### 槽

#### 调度器

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

#### 销毁器

#### 存储

提供了独占式的存储器和共享式的存储器。
共享式存储器支持写时复制，适合用在复制开销较大的场景
需要注意的是，这种写时复制只在使用 `=`时生效

#### around_ptr
