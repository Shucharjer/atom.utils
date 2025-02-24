#include "reflection.hpp"
#include "require.hpp"

using namespace atom;

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
        REQUIRES(utils::member_count_of<aggregate>() == 2);
        REQUIRES(utils::member_count_of<not_aggregate>() == 2);
    }

    // get
    {
        const auto num = 114514;
        const char ch  = '!';
        auto a         = aggregate{ .member1 = num, .member2 = ch };
        auto na        = not_aggregate{ num, ch };

        REQUIRES(utils::get<0>(a) == num);
        REQUIRES(utils::get<1>(a) == ch);

        REQUIRES(utils::get<0>(na) == num);
        REQUIRES(utils::get<1>(na) == ch);
    }

    // member_names_of
    {
        auto names_a  = utils::member_names_of<aggregate>();
        auto names_na = utils::member_names_of<not_aggregate>();

        REQUIRES(names_a[0] == "member1");
        REQUIRES(names_a[1] == "member2");

        REQUIRES(names_na[0] == "another_member1");
        REQUIRES(names_na[1] == "another_member2");
    }

    // description_of
    {
        auto description_a  = utils::description_of<aggregate>();
        auto description_na = utils::description_of<not_aggregate>();

        REQUIRES(utils::authenticity_of(description_a, utils::bits::is_aggregate));
        REQUIRES(!utils::authenticity_of(description_na, utils::bits::is_aggregate));
    }

    // reflected
    {
        auto reflected_a  = utils::reflected<aggregate>();
        auto reflected_na = utils::reflected<not_aggregate>();

        using namespace utils::bits;

        auto description_a  = reflected_a.description();
        auto description_na = reflected_na.description();

        REQUIRES(utils::authenticity_of(description_a, is_aggregate))
        REQUIRES_FALSE(utils::authenticity_of(description_na, is_aggregate))
    }

    // offsets
    {
        auto offsets = utils::offsets_of<aggregate>();
        auto ptr = std::get<1>(offsets);
        auto a = aggregate{};
        a.member2 = 'c';
        REQUIRES(a.*ptr == 'c');
        a.*ptr = 'b';
        REQUIRES(a.member2 == 'b');
    }
}
