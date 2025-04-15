#include <iostream>
#include <core/poly.hpp>
#include <misc/timer.hpp>

using namespace atom::utils;

struct base : public invoke_list<void() const, void() const, void() const, void() const> {
    template <typename Base>
    struct interface : Base {
        void foo() const noexcept { return poly_call<0>(*this); }
        void foo2() const noexcept { return poly_call<1>(*this); }
        void foo3() const noexcept { return poly_call<2>(*this); }
        void foo4() const noexcept { return poly_call<3>(*this); }
    };

    template <typename Impl>
    using implementation = value_list<&Impl::foo, &Impl::foo2, &Impl::foo3, &Impl::foo4>;
};
struct derived {
    void foo() const noexcept {}
    void foo2() const noexcept {}
    void foo3() const noexcept {}
    void foo4() const noexcept {}
};
struct tri_base {
    virtual ~tri_base() {}
    virtual void foo() const noexcept  = 0;
    virtual void foo2() const noexcept = 0;
    virtual void foo3() const noexcept = 0;
    virtual void foo4() const noexcept = 0;
};
struct tri_derived : tri_base {
    ~tri_derived() override {}
    void foo() const noexcept override {}
    void foo2() const noexcept override {}
    void foo3() const noexcept override {}
    void foo4() const noexcept override {}
};
int main() {
    timer timer;
    auto& t              = timer["poly"];
    poly<base> poly      = derived{};
    constexpr auto count = 100000000;
    t.from_now();
    for (auto i = 0; i < count; ++i) {
        poly->foo4();
    }
    std::cout << t.to_now() << '\n';
    tri_base* base = new tri_derived;
    t.from_now();
    for (auto i = 0; i < count; ++i) {
        base->foo4();
    }
    std::cout << t.to_now() << '\n';
    delete base;
}
