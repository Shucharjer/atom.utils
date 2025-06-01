#pragma once

namespace atom::utils {

template <typename To, typename From>
struct same_constness {
    using type = To;
};
template <typename To, typename From>
struct same_constness<To, From const> {
    using type = To const;
};
template <typename To, typename From>
using same_constness_t = typename same_constness<To, From>::type;

template <typename To, typename From>
struct same_volatile {
    using type = To;
};

template <typename To, typename From>
struct same_volatile<To, volatile From> {
    using type = volatile To;
};

template <typename To, typename From>
using same_volatile_t = typename same_volatile<To, From>::type;

template <typename To, typename From>
struct same_cv {
    using type = To;
};

template <typename To, typename From>
struct same_cv<To, const From> {
    using type = const To;
};

template <typename To, typename From>
struct same_cv<To, volatile From> {
    using type = volatile To;
};

template <typename To, typename From>
struct same_cv<To, const volatile From> {
    using type = const volatile To;
};

template <typename To, typename From>
using same_cv_t = typename same_cv<To, From>::type;

template <typename To, typename From>
struct same_reference {
    using type = To;
};

template <typename To, typename From>
struct same_reference<To, From&> {
    using type = To&;
};

template <typename To, typename From>
struct same_reference<To, From&&> {
    using type = To&&;
};

template <typename To, typename From>
using same_reference_t = typename same_reference<To, From>::type;

template <typename To, typename From>
struct same_cvref {
    using type = To;
};

template <typename To, typename From>
struct same_cvref<To, From&> {
    using type = To&;
};

template <typename To, typename From>
struct same_cvref<To, From&&> {
    using type = To&&;
};

template <typename To, typename From>
struct same_cvref<To, const From> {
    using type = const To;
};

template <typename To, typename From>
struct same_cvref<To, const From&> {
    using type = const To&;
};

template <typename To, typename From>
struct same_cvref<To, const From&&> {
    using type = const To&&;
};

template <typename To, typename From>
struct same_cvref<To, volatile From> {
    using type = const To;
};

template <typename To, typename From>
struct same_cvref<To, volatile From&> {
    using type = const To&;
};

template <typename To, typename From>
struct same_cvref<To, volatile From&&> {
    using type = const To&&;
};

template <typename To, typename From>
struct same_cvref<To, const volatile From> {
    using type = const To;
};

template <typename To, typename From>
struct same_cvref<To, const volatile From&> {
    using type = const To&;
};

template <typename To, typename From>
struct same_cvref<To, const volatile From&&> {
    using type = const To&&;
};

template <typename To, typename From>
using same_cvref_t = typename same_cvref<To, From>::type;

} // namespace atom::utils
