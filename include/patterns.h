#pragma once

namespace atom::utils {

template <typename Derived>
class singleton {
public:
    using value_type = Derived;
    using self_type  = singleton;

    virtual ~singleton() = default;

    [[nodiscard]] static inline Derived& instance() {
        static Derived instance_;
        return instance_;
    }

    singleton(singleton const& other)            = delete;
    singleton& operator=(singleton const& other) = delete;
    singleton(singleton&&)                       = default;
    singleton& operator=(singleton&&)            = delete;

protected:
    singleton() = default;
};

} // namespace atom::utils
