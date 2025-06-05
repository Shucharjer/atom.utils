#ifndef ATOM_UTILS_NUMBERS_HPP
#define ATOM_UTILS_NUMBERS_HPP

namespace atom::utils::numbers {

namespace integral {

constexpr auto _2     = 0x2;
constexpr auto _4     = 0x4;
constexpr auto _8     = 0x8;
constexpr auto _16    = 0x10;
constexpr auto _32    = 0x20;
constexpr auto _64    = 0x40;
constexpr auto _128   = 0x80;
constexpr auto _256   = 0x100;
constexpr auto _512   = 0x200;
constexpr auto _1024  = 0x400;
constexpr auto _2048  = 0x800;
constexpr auto _4096  = 0x1000;
constexpr auto _8192  = 0x2000;
constexpr auto _16384 = 0x4000;
constexpr auto _32768 = 0x8000;
constexpr auto _65536 = 0x10000;

} // namespace integral

namespace floating {

constexpr auto _1_2   = 0.5F;
constexpr auto _1_2_d = 0.5;

} // namespace floating

} // namespace atom::utils::numbers

#endif
