#include "reflection.hpp"
#include <cstdlib>
#include <type_traits>
#include "require.hpp"

using namespace atom::utils;

struct aggregate {
    int member1;
    char member2;
};

class not_aggregate {
public:
    constexpr not_aggregate(int v1, char v2) noexcept : another_member1(v1), another_member2(v2) {}
    constexpr not_aggregate(const not_aggregate&)            = default;
    constexpr not_aggregate(not_aggregate&&)                 = default;
    constexpr not_aggregate& operator=(const not_aggregate&) = default;
    constexpr not_aggregate& operator=(not_aggregate&&)      = default;
    constexpr ~not_aggregate()                               = default;

    REFL_MEMBERS(not_aggregate, another_member1, another_member2)
private:
    int another_member1;
    char another_member2;
};

int main() {
    // member_count_of
    {
        REQUIRES(member_count_of<aggregate>() == 2);
        REQUIRES(member_count_of<not_aggregate>() == 2);
    }

    // get
    {
        const auto num = 114514;
        const char ch  = '!';
        auto a         = aggregate{ .member1 = num, .member2 = ch };
        auto na        = not_aggregate{ num, ch };

        REQUIRES(get<0>(a) == num);
        REQUIRES(get<1>(a) == ch);

        REQUIRES(get<0>(na) == num);
        REQUIRES(get<1>(na) == ch);
    }

    // member_names_of
    {
        auto names_a  = member_names_of<aggregate>();
        auto names_na = member_names_of<not_aggregate>();

        REQUIRES(names_a[0] == "member1");
        REQUIRES(names_a[1] == "member2");

        REQUIRES(names_na[0] == "another_member1");
        REQUIRES(names_na[1] == "another_member2");
    }

    // description_of
    {
        auto description_a  = description_of<aggregate>();
        auto description_na = description_of<not_aggregate>();

        using namespace bits;

        REQUIRES(authenticity_of({ .desc = description_a, .bits = is_aggregate }))
        REQUIRES(!authenticity_of({ .desc = description_na, .bits = is_aggregate }))
    }

    // reflected
    {
        auto reflected_a  = reflected<aggregate>();
        auto reflected_na = reflected<not_aggregate>();

        using namespace bits;

        auto description_a  = reflected_a.description();
        auto description_na = reflected_na.description();

        REQUIRES(authenticity_of({ .desc = description_a, .bits = is_aggregate }))
        REQUIRES_FALSE(authenticity_of({ .desc = description_na, .bits = is_aggregate }))
    }

    // offsets
    {
        auto offsets = offsets_of<aggregate>();
        auto ptr     = std::get<1>(offsets);
        auto a       = aggregate{};
        a.member2    = 'c';
        REQUIRES(a.*ptr == 'c')
        a.*ptr = 'b';
        REQUIRES(a.member2 == 'b')
    }

    // offset value
    {
        REQUIRES((offset_value_of<0, aggregate>() == 0))
        REQUIRES((offset_value_of<1, aggregate>() == 4))

        // TODO:
        // REQUIRES((offset_value_of<0, not_aggregate>() == 0))
        // REQUIRES((offset_value_of<1, not_aggregate>() == 4))
    }

    // member_type_of
    {
        REQUIRES((std::same_as<int, member_type_of_t<0, aggregate>>))
        REQUIRES((std::same_as<char, member_type_of_t<1, aggregate>>))

        REQUIRES((std::same_as<int, member_type_of_t<0, not_aggregate>>))
        REQUIRES((std::same_as<char, member_type_of_t<1, not_aggregate>>))
    }
}
