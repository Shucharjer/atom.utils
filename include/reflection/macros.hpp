#pragma once
#include "reflection.hpp"
#include "reflection/field_traits.hpp"
#include "reflection/function_traits.hpp"
#include "reflection/hash.hpp"
#include "reflection/reflected.hpp"
#include "reflection/register.hpp"

#define BEGIN_TYPE(type_name)                                                                      \
    template <                                                                                     \
        ::atom::utils::concepts::pure BaseConstexprExtend,                                         \
        template <::atom::utils::concepts::pure> typename ConstexprExtend>                         \
    struct ::atom::utils::reflected<type_name, BaseConstexprExtend, ConstexprExtend>               \
        : public ::atom::utils::basic_reflected<BaseConstexprExtend> {                             \
        using type = type_name;                                                                    \
        constexpr reflected()                                                                      \
            : ::atom::utils::basic_reflected<BaseConstexprExtend>(                                 \
                  #type_name, ::atom::utils::hash<type>()                                          \
              ) {}                                                                                 \
        //

#define FIELD(field_name)                                                                          \
    ::atom::utils::field_traits<decltype(&type::field_name)> { #field_name, &type::field_name }    \
    //

#define FIELDS(...)                                                                                \
private:                                                                                           \
    constexpr static auto fields_ = std::make_tuple(__VA_ARGS__);                                  \
                                                                                                   \
public:                                                                                            \
    constexpr auto& fields() const noexcept { return fields_; }

#define FUNC(funcname)                                                                             \
    ::atom::utils::function_traits<decltype(&type::funcname)> { #funcname, &type::funcname }       \
    //

#define FUNCS(...)                                                                                 \
private:                                                                                           \
    constexpr static auto funcs_ = std::make_tuple(__VA_ARGS__);                                   \
                                                                                                   \
public:                                                                                            \
    constexpr auto& funcs() const noexcept { return funcs_; }                                      \
    //

#define END_TYPE()                                                                                 \
    }                                                                                              \
    ;                                                                                              \
    //

#define REGISTER(type_name, register_name)                                                         \
    namespace _internal::type_registers {                                                          \
    static inline const ::atom::utils::type_register<                                              \
        type_name,                                                                                 \
        ::atom::utils::basic_constexpr_extend,                                                     \
        ::atom::utils::constexpr_extend>(register_name);                                           \
    }                                                                                              \
    //

// example:
// REFLECT(dummy_t, FIELD(field1), FIELD(field2), ...)
#define REFLECT(type_name, ...)                                                                    \
    constexpr auto reflect(const type_name*) {                                                     \
        using type = type_name;                                                                    \
        return std::make_tuple(#type_name, std::make_tuple(__VA_ARGS__));                          \
    }                                                                                              \
    //

// example:
// EXPOSE(dummy_t, FUNC(func1), FUNC(func2), ...)
#define EXPOSE(type_name, ...)                                                                     \
    constexpr auto expose(const type_name*) {                                                      \
        using type = type_name;                                                                    \
        return std::make_tuple(__VA_ARGS__);                                                       \
    }                                                                                              \
    //

BEGIN_TYPE(bool)
FIELDS()
FUNCS()
END_TYPE()

BEGIN_TYPE(char)
FIELDS()
FUNCS()
END_TYPE()

BEGIN_TYPE(wchar_t)
FIELDS()
FUNCS()
END_TYPE()

BEGIN_TYPE(uint8_t)
FIELDS()
FUNCS()
END_TYPE()

BEGIN_TYPE(int8_t)
FIELDS()
FUNCS()
END_TYPE()

BEGIN_TYPE(uint16_t)
FIELDS()
FUNCS()
END_TYPE()

BEGIN_TYPE(int16_t)
FIELDS()
FUNCS()
END_TYPE()

BEGIN_TYPE(uint32_t)
FIELDS()
FUNCS()
END_TYPE()

BEGIN_TYPE(int32_t)
FIELDS()
FUNCS()
END_TYPE()

BEGIN_TYPE(uint64_t)
FIELDS()
FUNCS()
END_TYPE()

BEGIN_TYPE(int64_t)
FIELDS()
FUNCS()
END_TYPE()
