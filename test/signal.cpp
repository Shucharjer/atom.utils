#include "signal.hpp"
#include <cassert>
#include "core.hpp"
#include "core/type.hpp"
#include "signal/delegate.hpp"
#include "signal/dispatcher.hpp"

using namespace atom;

static int foo(int) { return 114514; }
static auto foo2 = []() { return 1919810; };

int main() {
    utils::delegate<int(int)> delegate(utils::spread_arg<&foo>);
    utils::delegate<int(void)> delegate2(utils::spread_arg<foo2>);

    assert(delegate(int{}) == 114514);
    assert(delegate2() == 1919810);
}
