#include "signal.hpp"
#include <cassert>
#include "core.hpp"
#include "core/type.hpp"
#include "signal/delegate.hpp"
#include "signal/dispatcher.hpp"

using namespace atom::utils;

static int foo(int) { return 114514; }
static auto foo2 = []() { return 1919810; };

int main() {
    // delegate
    {
        delegate<int(int)> delegate1(spread_arg<&foo>);
        delegate<int()> delegate2(spread_arg<foo2>);

        assert(delegate1(int{}) == 114514);
        assert(delegate2() == 1919810);
    }
}
