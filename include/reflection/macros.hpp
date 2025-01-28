#pragma once
#include "reflection.hpp"
#include "reflection/field_traits.hpp"
#include "reflection/function_traits.hpp"
#include "reflection/hash.hpp"
#include "reflection/reflected.hpp"
#include "reflection/register.hpp"

#define BEGIN_TYPE(type_name)                                                                      \
    template <                                                                                     \
        UCONCEPTS pure BaseConstexprExtend,                                                        \
        template <UCONCEPTS pure> typename ConstexprExtend>                                        \
    struct ::atom::utils::reflected<type_name, BaseConstexprExtend, ConstexprExtend>               \
        : public ::atom::utils::basic_reflected<BaseConstexprExtend> {                             \
        using type = type_name;                                                                    \
        constexpr reflected()                                                                      \
            : ::atom::utils::basic_reflected<BaseConstexprExtend>(                                 \
                  #type_name, ::atom::utils::hash<type_name>()                                     \
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
    static inline ::atom::utils::type_register<                                                    \
        type_name,                                                                                 \
        ::atom::utils::basic_constexpr_extend,                                                     \
        ::atom::utils::constexpr_extend>(register_name);                                           \
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
