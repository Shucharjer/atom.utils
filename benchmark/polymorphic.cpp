#include <iostream>
#include <benchmark/benchmark.h>
#include <core/polymorphic.hpp>

using namespace atom::utils;

struct Base {
    virtual ~Base() = default;
    virtual void foo() const {
#ifdef _OUTPUT
        std::cout << "Base::foo called\n";
#endif
    }
};

struct Derived : public Base {
    ~Derived() override = default;
    void foo() const override {
#ifdef _OUTPUT
        std::cout << "Derived::foo called\n";
#endif
    }
};

static void BM_derive(benchmark::State& state) {
    Base* base = new Derived();
    for (auto _ : state) {
        base->foo();
    }
    delete base;
}

struct Object {
    template <typename Base>
    struct interface : public Base {
        void foo() const { this->template invoke<0>(); }
    };
    template <typename Impl>
    using impl = value_list<&Impl::foo>;
};

struct Impl {
    void foo() const {
#ifdef _OUTPUT
        std::cout << "Impl::foo called\n";
#endif
    }
};

static void BM_vtable(benchmark::State& state) {
    auto* _vtable = new vtable_t<Object>();
    auto& vtable  = *_vtable;
    vtable        = make_vtable_tuple<Object, Impl>();
    for (auto _ : state) {
        std::get<0>(vtable)(nullptr);
    }
    delete _vtable;
}

static void BM_polymorphic(benchmark::State& state) {
    auto* _poly = new polymorphic<Object, 8, 8>(Impl{});
    auto& poly  = *_poly;
    for (auto _ : state) {
        poly->foo();
    }
    delete _poly;
}

BENCHMARK(BM_derive);
BENCHMARK(BM_vtable);
BENCHMARK(BM_polymorphic);

BENCHMARK_MAIN();
