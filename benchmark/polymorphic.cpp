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
        void foo() const { call_polymorphic<0>(this); }
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
    auto* _vtable          = new vtable<Object>();
    vtable<Object>& vtable = *_vtable;
    vtable.value           = _vtable_tuple_value<Object, Impl, void*>();
    for (auto _ : state) {
        vtable.invoke<0>(nullptr);
        std::any a;
    }
    delete _vtable;
}

static void BM_polymorphic(benchmark::State& state) {
    auto* _poly               = new polymorphic<Object>(Impl{});
    polymorphic<Object>& poly = *_poly;
    for (auto _ : state) {
        poly->foo();
    }
    delete _poly;
}

BENCHMARK(BM_derive);
BENCHMARK(BM_vtable);
BENCHMARK(BM_polymorphic);

BENCHMARK_MAIN();
