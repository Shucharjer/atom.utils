#include "core.hpp"
#include <algorithm>
#include <cassert>
#include <format>
#include <initializer_list>
#include <ranges>
#include "core/closure.hpp"
#include "core/pipeline.hpp"
#include "core/polymorphic.hpp"
#include "output.hpp"

using namespace atom::utils;

struct get_vector_fn {
    template <typename... Args>
    std::vector<int> operator()(Args&&... args) {
        return std::vector<int>(std::forward<Args>(args)...);
    }
};

struct empty_fn {
    using pipeline_tag = atom::utils::pipeline_tag;

    template <std::ranges::range Rng>
    Rng operator()(const Rng& range) const {
        return Rng{};
    }
};

constexpr inline empty_fn empty;

// struct basic_object : invoke_list<void() const, void(), void(int)> {
//     template <typename Base>
//     struct interface : Base {
//         void foo() const { return poly_call<0>(*this); }
//         void foo2() { return poly_call<1>(*this); }
//         void foo3(int i) { return poly_call<2>(*this, i); }
//     };

//     template <typename Impl>
//     using implementation = value_list<&Impl::foo, &Impl::foo2, &Impl::foo3>;
// };

// struct advanced_object
//     : append_invoke_list_t<decltype(as_invoke_list(std::declval<basic_object>())), void()> {
//     template <typename Base>
//     struct interface : basic_object::interface<Base> {
//         void foo4() { return poly_call<3>(*this); }
//     };

//     template <typename Impl>
//     using implementation = append_value_list_t<basic_object::implementation<Impl>, &Impl::foo4>;
// };

struct Object {
    template <typename Base>
    struct interface : Base {
        void foo() const { call_polymorphic<0>(this); }
    };

    template <typename Impl>
    using impl = value_list<&Impl::foo>;
};

int main() {
    // polymorphic
    {
        struct _impl {
            void foo() const { println("called foo() in _impl"); }
        };

        polymorphic<Object> poly{ _impl{} };
        poly->foo();
    }
    // poly
    // {
    //     struct basic_impl {
    //         void foo() const { println("called foo() in basic_impl"); }
    //         void foo2() { println("called foo2() in basic_impl"); }
    //         void foo3(int i) { println(std::format("called foo3() in basic_impl, i: {}", i)); }
    //     };

    //     struct advanced_impl : basic_impl {
    //         void foo2() { println("called foo2() in advanced_impl"); }
    //         void foo4() { println(std::format("called foo4() in avanced_impl")); }
    //     };

    //     poly<basic_object> basic_poly = basic_impl{};
    //     println("basic_impl:");
    //     basic_poly->foo();
    //     basic_poly->foo2();
    //     newline();

    //     poly<advanced_object> advanced_poly = advanced_impl{};
    //     println("advanced_impl:");
    //     advanced_poly->foo();
    //     advanced_poly->foo2();
    //     advanced_poly->foo4();
    //     newline();
    // }
    // pair
    {
        auto cpair   = compressed_pair<int, int>{};
        auto& rcpair = reverse(cpair);
        auto another = compressed_pair<int, std::string>{};
        // cause error: std::string is not trivial type.
        // auto& another_reversed = utils::reverse(another);

        auto pcpair      = pair<char, int>{};
        auto& [pcf, pcs] = pcpair;

        compressed_pair<int, int> third{ cpair.first(), cpair.second() };
    }
    // pipeline
    {
        auto construct_arg = 10;
        auto get_vector    = make_closure<get_vector_fn>();
        auto result        = get_vector(construct_arg);
        assert(result.size() == construct_arg);

        std::vector vector = { 2, 3, 4, 6 };
        auto empty_vector  = vector | empty;
        assert(empty_vector.empty());

        auto closure              = empty | std::views::reverse;
        auto another_closure      = closure | std::views::reverse;
        auto another_empty_vector = vector | closure;
        assert(another_empty_vector.empty());
    }
    // closure
    {
        std::vector origin = { 2, 2, 3, 34, 2, 523, 53, 5, 346, 54, 645, 7, 4567, 56, 75 };
        auto end           = origin.cend();
        auto closure       = make_closure<get_vector_fn>(end);

        auto vec  = closure(origin.cbegin() + 2);
        auto vec2 = closure(origin.cbegin() + 3);
        auto vec3 = (origin.cbegin() + 3) | closure;
        std::ranges::for_each(vec, print);
        newline();
    }

    return 0;
}
