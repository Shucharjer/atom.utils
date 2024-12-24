#pragma once
#include "compressed_pair.h"

namespace atom::utils {

template <typename First, typename Second>
class reversed_compressed_pair final : private internal::compressed_element<First, false>, private internal::compressed_element<Second, true> {
    // TODO:
};

template <typename First, typename Second>
struct reversed_pair {
    Second first;
    First second;
};

}
