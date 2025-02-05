#ifdef TEST_UTILS
    #include "reflection.hpp"
    #include <catch.hpp>
    #include "output.hpp"
    #include "reflection/macros.hpp"
    #include "reflection/registery.hpp"

using namespace atom;

struct reflected_class {
    int member1;
    char member2;
};

BEGIN_TYPE(reflected_class)
FIELDS(FIELD(member1), FIELD(member2))
END_TYPE()
REGISTER(reflected_class, reflected_class_register)

TEST_CASE("reflection") {
    SECTION("reflected") {
        auto inst      = reflected_class{};
        auto reflected = utils::reflected<reflected_class>{};
        REQUIRE(reflected.name() == "reflected_class");

        auto& fields  = reflected.fields();
        auto& traits1 = std::get<0>(fields);
        REQUIRE(traits1.name() == "member1");

        const auto test_num = 114514;
        inst.member1        = test_num;
        REQUIRE(traits1.get(inst) == test_num);

        constexpr auto index = utils::index_of<"member1">(fields);
        REQUIRE(index == 0);
    }

    SECTION("registry") {
        SECTION("all") {
            auto&& reflecteds =
                utils::registry<utils::basic_constexpr_extend, utils::constexpr_extend>::all();
            auto takes = reflecteds | std::views::filter([](const auto& reflected) {
                             return reflected->name() == "reflected_class";
                         });
            REQUIRE_FALSE(takes.empty());
        }

        SECTION("identity") {
            const auto hash         = utils::hash<reflected_class>();
            const auto de_reflected = utils::reflected<reflected_class>();
            print(de_reflected.name());
            newline();
            REQUIRE(de_reflected.hash() == hash);

            const auto identity   = utils::basic_registry::identity(hash);
            const auto& reflected = utils::basic_registry::find<reflected_class>();
            REQUIRE(hash == reflected->hash());
        }
    }
}

#endif
