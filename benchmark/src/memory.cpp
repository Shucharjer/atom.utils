#include <memory_resource>
#include <benchmark/benchmark.h>
#include <core.hpp>
#include <memory.hpp>

using namespace atom::utils;

struct ATOM_NOVTABLE allocator_base {
    allocator_base()                                 = default;
    allocator_base(const allocator_base&)            = default;
    allocator_base(allocator_base&&)                 = default;
    allocator_base& operator=(const allocator_base&) = default;
    allocator_base& operator=(allocator_base&&)      = default;
    virtual ~allocator_base()                        = default;
    virtual void* allocate()                         = 0;
    virtual void deallocate(void* ptr)               = 0;
};

template <typename Ty>
struct pmrallocator : allocator_base, private std::pmr::polymorphic_allocator<Ty> {

    pmrallocator() : std::pmr::polymorphic_allocator<Ty>(std::pmr::get_default_resource()) {}

    template <typename Al>
    requires std::constructible_from<std::pmr::polymorphic_allocator<Ty>, Al>
    pmrallocator(const Al& alloc) : std::pmr::polymorphic_allocator<Ty>(alloc) {}

    pmrallocator(const pmrallocator&)            = default;
    pmrallocator(pmrallocator&&)                 = default;
    pmrallocator& operator=(const pmrallocator&) = default;
    pmrallocator& operator=(pmrallocator&&)      = default;

    ~pmrallocator() override = default;

    void* allocate() override {
        return static_cast<std::pmr::polymorphic_allocator<Ty>*>(this)->allocate(1);
    }
    void deallocate(void* ptr) override {
        static_cast<std::pmr::polymorphic_allocator<Ty>*>(this)->deallocate(
            static_cast<Ty*>(ptr), 1);
    }
};

// NOLINTBEGIN(cppcoreguidelines-avoid-non-const-global-variables)

static inline std::pmr::polymorphic_allocator<void> alloc;

// NOLINTEND(cppcoreguidelines-avoid-non-const-global-variables)

static void BM_allocator_base(benchmark::State& state) {
    allocator_base* allocator = new pmrallocator<int>(alloc);
    for (auto _ : state) {
        auto* ptr = allocator->allocate();
        allocator->deallocate(ptr);
    }
    delete allocator;
}

BENCHMARK(BM_allocator_base);

static void BM_common_allocator(benchmark::State& state) {
    auto allocator = make_common_allocator<int, std::pmr::polymorphic_allocator<int>>(alloc);
    for (auto _ : state) {
        auto* ptr = allocator.allocate();
        allocator.deallocate(ptr);
    }
}

BENCHMARK(BM_common_allocator);

static void BM_common_tiny_allocator(benchmark::State& state) {
    auto allocator = make_common_tiny_allocator<int, std::pmr::polymorphic_allocator>(alloc);
    for (auto _ : state) {
        auto* ptr = allocator.allocate();
        allocator.deallocate(ptr);
    }
}

BENCHMARK(BM_common_tiny_allocator);

template <typename Ty, typename Alloc>
struct Allocator : private Alloc {
    using Alloc::Alloc;
    [[nodiscard]] void* allocate() { return static_cast<Alloc*>(this)->allocate(1); }
    void deallocate(void* ptr) { static_cast<Alloc*>(this)->deallocate(static_cast<Ty*>(ptr), 1); }
};

static void BM_polymorphic_allocator(benchmark::State& state) {
    poly<allocator_object> allocator{ Allocator<int, std::pmr::polymorphic_allocator<int>>{
        alloc } };
    for (auto _ : state) {
        auto* ptr = allocator->allocate();
        allocator->deallocate(ptr);
    }
}

BENCHMARK(BM_polymorphic_allocator);

#if __cplusplus > 202002L
    #include "proxy.hpp"

PRO_DEF_MEM_DISPATCH(MemAllocate, allocate);
PRO_DEF_MEM_DISPATCH(MemDeallocate, deallocate);

struct proxy_alloc
    : pro::facade_builder::add_convention<MemAllocate, void*()>::add_convention<
          MemDeallocate, void(void*)>::support_copy<pro::constraint_level::trivial>::build {};
template <typename Ty, typename Alloc = std::pmr::polymorphic_allocator<Ty>>
class proxy_allocator : private Alloc {
public:
    using Alloc::Alloc;
    void* allocate() { return static_cast<Alloc*>(this)->allocate(1); }
    void deallocate(void* ptr) { static_cast<Alloc*>(this)->deallocate(static_cast<Ty*>(ptr), 1); }
};

static void BM_proxy_allocator(benchmark::State& state) {
    auto p =
        pro::make_proxy<proxy_alloc, proxy_allocator<int, std::pmr::polymorphic_allocator<int>>>();
    for (auto _ : state) {
        auto* ptr = p->allocate();
        p->deallocate(ptr);
    }
}

BENCHMARK(BM_proxy_allocator);

#endif

// NOLINTBEGIN(cppcoreguidelines-avoid-c-arrays)
// NOLINTBEGIN(modernize-avoid-c-arrays)
// NOLINTBEGIN(cppcoreguidelines-pro-type-reinterpret-cast)

BENCHMARK_MAIN();

// NOLINTEND(cppcoreguidelines-pro-type-reinterpret-cast)
// NOLINTEND(modernize-avoid-c-arrays)
// NOLINTEND(cppcoreguidelines-avoid-c-arrays)
