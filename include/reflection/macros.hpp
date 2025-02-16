#pragma once
#include "reflection/field_traits.hpp"
#include "reflection/function_traits.hpp"

#define REGISTER(type_name, register_name)                                                         \
    namespace _internal::type_registers {                                                          \
    static inline const ::atom::utils::type_register<type_name>(register_name);                    \
    }                                                                                              \
    //

// NOLINTBEGIN(cppcoreguidelines-macro-usage)
#define NUM_ARGS_HELPER_(                                                                          \
    _1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15, _16, _17, _18, _19, _20,     \
    _21, _22, _23, _24, _25, _26, _27, _28, _29, _30, _31, _32, _33, _34, _35, _36, _37, _38, _39, \
    _40, _41, _42, _43, _44, _45, _46, _47, _48, _49, _50, _51, _52, _53, _54, _55, _56, _57, _58, \
    _59, _60, _61, _62, _63, _64, _65, _66, _67, _68, _69, _70, _71, _72, _73, _74, _75, _76, _77, \
    _78, _79, _80, _81, _82, _83, _84, _85, _86, _87, _88, _89, _90, _91, _92, _93, _94, _95, _96, \
    _97, _98, _99, _100, _101, _102, _103, _104, _105, _106, _107, _108, _109, _110, _111, _112,   \
    _113, _114, _115, _116, _117, _118, _119, _120, _121, _122, _123, _124, _125, _126, _127,      \
    _128, _129, _130, _131, _132, _133, _134, _135, _136, _137, _138, _139, _140, _141, _142,      \
    _143, _144, _145, _146, _147, _148, _149, _150, _151, _152, _153, _154, _155, _156, _157,      \
    _158, _159, _160, _161, _162, _163, _164, _165, _166, _167, _168, _169, _170, _171, _172,      \
    _173, _174, _175, _176, _177, _178, _179, _180, _181, _182, _183, _184, _185, _186, _187,      \
    _188, _189, _190, _191, _192, _193, _194, _195, _196, _197, _198, _199, _200, _201, _202,      \
    _203, _204, _205, _206, _207, _208, _209, _210, _211, _212, _213, _214, _215, _216, _217,      \
    _218, _219, _220, _221, _222, _223, _224, _225, _226, _227, _228, _229, _230, _231, _232,      \
    _233, _234, _235, _236, _237, _238, _239, _240, _241, _242, _243, _244, _245, _246, _247,      \
    _248, _249, _250, _251, _252, _253, _254, _255, N, ...)                                        \
    N

#define NUM_ARGS_HELPER(...)                                                                       \
    NUM_ARGS_HELPER_(                                                                              \
        __VA_ARGS__, 255, 254, 253, 252, 251, 250, 249, 248, 247, 246, 245, 244, 243, 242, 241,    \
        240, 239, 238, 237, 236, 235, 234, 233, 232, 231, 230, 229, 228, 227, 226, 225, 224, 223,  \
        222, 221, 220, 219, 218, 217, 216, 215, 214, 213, 212, 211, 210, 209, 208, 207, 206, 205,  \
        204, 203, 202, 201, 200, 199, 198, 197, 196, 195, 194, 193, 192, 191, 190, 189, 188, 187,  \
        186, 185, 184, 183, 182, 181, 180, 179, 178, 177, 176, 175, 174, 173, 172, 171, 170, 169,  \
        168, 167, 166, 165, 164, 163, 162, 161, 160, 159, 158, 157, 156, 155, 154, 153, 152, 151,  \
        150, 149, 148, 147, 146, 145, 144, 143, 142, 141, 140, 139, 138, 137, 136, 135, 134, 133,  \
        132, 131, 130, 129, 128, 127, 126, 125, 124, 123, 122, 121, 120, 119, 118, 117, 116, 115,  \
        114, 113, 112, 111, 110, 109, 108, 107, 106, 105, 104, 103, 102, 101, 100, 99, 98, 97, 96, \
        95, 94, 93, 92, 91, 90, 89, 88, 87, 86, 85, 84, 83, 82, 81, 80, 79, 78, 77, 76, 75, 74,    \
        73, 72, 71, 70, 69, 68, 67, 66, 65, 64, 63, 62, 61, 60, 59, 58, 57, 56, 55, 54, 53, 52,    \
        51, 50, 49, 48, 47, 46, 45, 44, 43, 42, 41, 40, 39, 38, 37, 36, 35, 34, 33, 32, 31, 30,    \
        29, 28, 27, 26, 25, 24, 23, 22, 21, 20, 19, 18, 17, 16, 15, 14, 13, 12, 11, 10, 9, 8, 7,   \
        6, 5, 4, 3, 2, 1)

#define NUM_ARGS(...) NUM_ARGS_HELPER(__VA_ARGS__)

// NOLINTEND(cppcoreguidelines-macro-usage)

#define CONCAT_(l, r) l##r
#define CONCAT(l, r) CONCAT_(l, r)

#define FIELD_TRAITS(_, x)                                                                         \
    ::atom::utils::field_traits<decltype(&_::x)> { #x, &_::x }                                     \
    //

#define FUNCTION_TRAITS(_, x)                                                                      \
    ::atom::utils::function_traits<decltype(&_::x)> { #x, &_::x }                                  \
    //

#define EXPAND(...) __VA_ARGS__
#define FOR_EACH_0(MACRO, _)
#define FOR_EACH_1(MACRO, _, _1) MACRO(_, _1)
#define FOR_EACH_2(MACRO, _, _1, ...) MACRO(_, _1), EXPAND(FOR_EACH_1(MACRO, _, __VA_ARGS__))
#define FOR_EACH_3(MACRO, _, _1, ...) MACRO(_, _1), EXPAND(FOR_EACH_2(MACRO, _, __VA_ARGS__))
#define FOR_EACH_4(MACRO, _, _1, ...) MACRO(_, _1), EXPAND(FOR_EACH_3(MACRO, _, __VA_ARGS__))
#define FOR_EACH_5(MACRO, _, _1, ...) MACRO(_, _1), EXPAND(FOR_EACH_4(MACRO, _, __VA_ARGS__))
#define FOR_EACH_6(MACRO, _, _1, ...) MACRO(_, _1), EXPAND(FOR_EACH_5(MACRO, _, __VA_ARGS__))
#define FOR_EACH_7(MACRO, _, _1, ...) MACRO(_, _1), EXPAND(FOR_EACH_6(MACRO, _, __VA_ARGS__))
#define FOR_EACH_8(MACRO, _, _1, ...) MACRO(_, _1), EXPAND(FOR_EACH_7(MACRO, _, __VA_ARGS__))
#define FOR_EACH_9(MACRO, _, _1, ...) MACRO(_, _1), EXPAND(FOR_EACH_8(MACRO, _, __VA_ARGS__))
#define FOR_EACH_10(MACRO, _, _1, ...) MACRO(_, _1), EXPAND(FOR_EACH_9(MACRO, _, __VA_ARGS__))
#define FOR_EACH_11(MACRO, _, _1, ...) MACRO(_, _1), EXPAND(FOR_EACH_10(MACRO, _, __VA_ARGS__))
#define FOR_EACH_12(MACRO, _, _1, ...) MACRO(_, _1), EXPAND(FOR_EACH_11(MACRO, _, __VA_ARGS__))
#define FOR_EACH_13(MACRO, _, _1, ...) MACRO(_, _1), EXPAND(FOR_EACH_12(MACRO, _, __VA_ARGS__))
#define FOR_EACH_14(MACRO, _, _1, ...) MACRO(_, _1), EXPAND(FOR_EACH_13(MACRO, _, __VA_ARGS__))
#define FOR_EACH_15(MACRO, _, _1, ...) MACRO(_, _1), EXPAND(FOR_EACH_14(MACRO, _, __VA_ARGS__))
#define FOR_EACH_16(MACRO, _, _1, ...) MACRO(_, _1), EXPAND(FOR_EACH_15(MACRO, _, __VA_ARGS__))
#define FOR_EACH_17(MACRO, _, _1, ...) MACRO(_, _1), EXPAND(FOR_EACH_16(MACRO, _, __VA_ARGS__))
#define FOR_EACH_18(MACRO, _, _1, ...) MACRO(_, _1), EXPAND(FOR_EACH_17(MACRO, _, __VA_ARGS__))
#define FOR_EACH_19(MACRO, _, _1, ...) MACRO(_, _1), EXPAND(FOR_EACH_18(MACRO, _, __VA_ARGS__))
#define FOR_EACH_20(MACRO, _, _1, ...) MACRO(_, _1), EXPAND(FOR_EACH_19(MACRO, _, __VA_ARGS__))
#define FOR_EACH_21(MACRO, _, _1, ...) MACRO(_, _1), EXPAND(FOR_EACH_20(MACRO, _, __VA_ARGS__))
#define FOR_EACH_22(MACRO, _, _1, ...) MACRO(_, _1), EXPAND(FOR_EACH_21(MACRO, _, __VA_ARGS__))
#define FOR_EACH_23(MACRO, _, _1, ...) MACRO(_, _1), EXPAND(FOR_EACH_22(MACRO, _, __VA_ARGS__))
#define FOR_EACH_24(MACRO, _, _1, ...) MACRO(_, _1), EXPAND(FOR_EACH_23(MACRO, _, __VA_ARGS__))
#define FOR_EACH_25(MACRO, _, _1, ...) MACRO(_, _1), EXPAND(FOR_EACH_24(MACRO, _, __VA_ARGS__))
#define FOR_EACH_26(MACRO, _, _1, ...) MACRO(_, _1), EXPAND(FOR_EACH_25(MACRO, _, __VA_ARGS__))
#define FOR_EACH_27(MACRO, _, _1, ...) MACRO(_, _1), EXPAND(FOR_EACH_26(MACRO, _, __VA_ARGS__))
#define FOR_EACH_28(MACRO, _, _1, ...) MACRO(_, _1), EXPAND(FOR_EACH_27(MACRO, _, __VA_ARGS__))
#define FOR_EACH_29(MACRO, _, _1, ...) MACRO(_, _1), EXPAND(FOR_EACH_28(MACRO, _, __VA_ARGS__))
#define FOR_EACH_30(MACRO, _, _1, ...) MACRO(_, _1), EXPAND(FOR_EACH_29(MACRO, _, __VA_ARGS__))
#define FOR_EACH_31(MACRO, _, _1, ...) MACRO(_, _1), EXPAND(FOR_EACH_30(MACRO, _, __VA_ARGS__))
#define FOR_EACH_32(MACRO, _, _1, ...) MACRO(_, _1), EXPAND(FOR_EACH_31(MACRO, _, __VA_ARGS__))
#define FOR_EACH_33(MACRO, _, _1, ...) MACRO(_, _1), EXPAND(FOR_EACH_32(MACRO, _, __VA_ARGS__))
#define FOR_EACH_34(MACRO, _, _1, ...) MACRO(_, _1), EXPAND(FOR_EACH_33(MACRO, _, __VA_ARGS__))
#define FOR_EACH_35(MACRO, _, _1, ...) MACRO(_, _1), EXPAND(FOR_EACH_34(MACRO, _, __VA_ARGS__))
#define FOR_EACH_36(MACRO, _, _1, ...) MACRO(_, _1), EXPAND(FOR_EACH_35(MACRO, _, __VA_ARGS__))
#define FOR_EACH_37(MACRO, _, _1, ...) MACRO(_, _1), EXPAND(FOR_EACH_36(MACRO, _, __VA_ARGS__))
#define FOR_EACH_38(MACRO, _, _1, ...) MACRO(_, _1), EXPAND(FOR_EACH_37(MACRO, _, __VA_ARGS__))
#define FOR_EACH_39(MACRO, _, _1, ...) MACRO(_, _1), EXPAND(FOR_EACH_38(MACRO, _, __VA_ARGS__))
#define FOR_EACH_40(MACRO, _, _1, ...) MACRO(_, _1), EXPAND(FOR_EACH_39(MACRO, _, __VA_ARGS__))
#define FOR_EACH_41(MACRO, _, _1, ...) MACRO(_, _1), EXPAND(FOR_EACH_40(MACRO, _, __VA_ARGS__))
#define FOR_EACH_42(MACRO, _, _1, ...) MACRO(_, _1), EXPAND(FOR_EACH_41(MACRO, _, __VA_ARGS__))
#define FOR_EACH_43(MACRO, _, _1, ...) MACRO(_, _1), EXPAND(FOR_EACH_42(MACRO, _, __VA_ARGS__))
#define FOR_EACH_44(MACRO, _, _1, ...) MACRO(_, _1), EXPAND(FOR_EACH_43(MACRO, _, __VA_ARGS__))
#define FOR_EACH_45(MACRO, _, _1, ...) MACRO(_, _1), EXPAND(FOR_EACH_44(MACRO, _, __VA_ARGS__))
#define FOR_EACH_46(MACRO, _, _1, ...) MACRO(_, _1), EXPAND(FOR_EACH_45(MACRO, _, __VA_ARGS__))
#define FOR_EACH_47(MACRO, _, _1, ...) MACRO(_, _1), EXPAND(FOR_EACH_46(MACRO, _, __VA_ARGS__))
#define FOR_EACH_48(MACRO, _, _1, ...) MACRO(_, _1), EXPAND(FOR_EACH_47(MACRO, _, __VA_ARGS__))
#define FOR_EACH_49(MACRO, _, _1, ...) MACRO(_, _1), EXPAND(FOR_EACH_48(MACRO, _, __VA_ARGS__))
#define FOR_EACH_50(MACRO, _, _1, ...) MACRO(_, _1), EXPAND(FOR_EACH_49(MACRO, _, __VA_ARGS__))
#define FOR_EACH_51(MACRO, _, _1, ...) MACRO(_, _1), EXPAND(FOR_EACH_50(MACRO, _, __VA_ARGS__))
#define FOR_EACH_52(MACRO, _, _1, ...) MACRO(_, _1), EXPAND(FOR_EACH_51(MACRO, _, __VA_ARGS__))
#define FOR_EACH_53(MACRO, _, _1, ...) MACRO(_, _1), EXPAND(FOR_EACH_52(MACRO, _, __VA_ARGS__))
#define FOR_EACH_54(MACRO, _, _1, ...) MACRO(_, _1), EXPAND(FOR_EACH_53(MACRO, _, __VA_ARGS__))
#define FOR_EACH_55(MACRO, _, _1, ...) MACRO(_, _1), EXPAND(FOR_EACH_54(MACRO, _, __VA_ARGS__))
#define FOR_EACH_56(MACRO, _, _1, ...) MACRO(_, _1), EXPAND(FOR_EACH_55(MACRO, _, __VA_ARGS__))
#define FOR_EACH_57(MACRO, _, _1, ...) MACRO(_, _1), EXPAND(FOR_EACH_56(MACRO, _, __VA_ARGS__))
#define FOR_EACH_58(MACRO, _, _1, ...) MACRO(_, _1), EXPAND(FOR_EACH_57(MACRO, _, __VA_ARGS__))
#define FOR_EACH_59(MACRO, _, _1, ...) MACRO(_, _1), EXPAND(FOR_EACH_58(MACRO, _, __VA_ARGS__))
#define FOR_EACH_60(MACRO, _, _1, ...) MACRO(_, _1), EXPAND(FOR_EACH_59(MACRO, _, __VA_ARGS__))
#define FOR_EACH_61(MACRO, _, _1, ...) MACRO(_, _1), EXPAND(FOR_EACH_60(MACRO, _, __VA_ARGS__))
#define FOR_EACH_62(MACRO, _, _1, ...) MACRO(_, _1), EXPAND(FOR_EACH_61(MACRO, _, __VA_ARGS__))
#define FOR_EACH_63(MACRO, _, _1, ...) MACRO(_, _1), EXPAND(FOR_EACH_62(MACRO, _, __VA_ARGS__))
#define FOR_EACH_64(MACRO, _, _1, ...) MACRO(_, _1), EXPAND(FOR_EACH_63(MACRO, _, __VA_ARGS__))
#define FOR_EACH_65(MACRO, _, _1, ...) MACRO(_, _1), EXPAND(FOR_EACH_64(MACRO, _, __VA_ARGS__))
#define FOR_EACH_66(MACRO, _, _1, ...) MACRO(_, _1), EXPAND(FOR_EACH_65(MACRO, _, __VA_ARGS__))
#define FOR_EACH_67(MACRO, _, _1, ...) MACRO(_, _1), EXPAND(FOR_EACH_66(MACRO, _, __VA_ARGS__))
#define FOR_EACH_68(MACRO, _, _1, ...) MACRO(_, _1), EXPAND(FOR_EACH_67(MACRO, _, __VA_ARGS__))
#define FOR_EACH_69(MACRO, _, _1, ...) MACRO(_, _1), EXPAND(FOR_EACH_68(MACRO, _, __VA_ARGS__))
#define FOR_EACH_70(MACRO, _, _1, ...) MACRO(_, _1), EXPAND(FOR_EACH_69(MACRO, _, __VA_ARGS__))
#define FOR_EACH_71(MACRO, _, _1, ...) MACRO(_, _1), EXPAND(FOR_EACH_70(MACRO, _, __VA_ARGS__))
#define FOR_EACH_72(MACRO, _, _1, ...) MACRO(_, _1), EXPAND(FOR_EACH_71(MACRO, _, __VA_ARGS__))
#define FOR_EACH_73(MACRO, _, _1, ...) MACRO(_, _1), EXPAND(FOR_EACH_72(MACRO, _, __VA_ARGS__))
#define FOR_EACH_74(MACRO, _, _1, ...) MACRO(_, _1), EXPAND(FOR_EACH_73(MACRO, _, __VA_ARGS__))
#define FOR_EACH_75(MACRO, _, _1, ...) MACRO(_, _1), EXPAND(FOR_EACH_74(MACRO, _, __VA_ARGS__))
#define FOR_EACH_76(MACRO, _, _1, ...) MACRO(_, _1), EXPAND(FOR_EACH_75(MACRO, _, __VA_ARGS__))
#define FOR_EACH_77(MACRO, _, _1, ...) MACRO(_, _1), EXPAND(FOR_EACH_76(MACRO, _, __VA_ARGS__))
#define FOR_EACH_78(MACRO, _, _1, ...) MACRO(_, _1), EXPAND(FOR_EACH_77(MACRO, _, __VA_ARGS__))
#define FOR_EACH_79(MACRO, _, _1, ...) MACRO(_, _1), EXPAND(FOR_EACH_78(MACRO, _, __VA_ARGS__))
#define FOR_EACH_80(MACRO, _, _1, ...) MACRO(_, _1), EXPAND(FOR_EACH_79(MACRO, _, __VA_ARGS__))
#define FOR_EACH_81(MACRO, _, _1, ...) MACRO(_, _1), EXPAND(FOR_EACH_80(MACRO, _, __VA_ARGS__))
#define FOR_EACH_82(MACRO, _, _1, ...) MACRO(_, _1), EXPAND(FOR_EACH_81(MACRO, _, __VA_ARGS__))
#define FOR_EACH_83(MACRO, _, _1, ...) MACRO(_, _1), EXPAND(FOR_EACH_82(MACRO, _, __VA_ARGS__))
#define FOR_EACH_84(MACRO, _, _1, ...) MACRO(_, _1), EXPAND(FOR_EACH_83(MACRO, _, __VA_ARGS__))
#define FOR_EACH_85(MACRO, _, _1, ...) MACRO(_, _1), EXPAND(FOR_EACH_84(MACRO, _, __VA_ARGS__))
#define FOR_EACH_86(MACRO, _, _1, ...) MACRO(_, _1), EXPAND(FOR_EACH_85(MACRO, _, __VA_ARGS__))
#define FOR_EACH_87(MACRO, _, _1, ...) MACRO(_, _1), EXPAND(FOR_EACH_86(MACRO, _, __VA_ARGS__))
#define FOR_EACH_88(MACRO, _, _1, ...) MACRO(_, _1), EXPAND(FOR_EACH_87(MACRO, _, __VA_ARGS__))
#define FOR_EACH_89(MACRO, _, _1, ...) MACRO(_, _1), EXPAND(FOR_EACH_88(MACRO, _, __VA_ARGS__))
#define FOR_EACH_90(MACRO, _, _1, ...) MACRO(_, _1), EXPAND(FOR_EACH_89(MACRO, _, __VA_ARGS__))
#define FOR_EACH_91(MACRO, _, _1, ...) MACRO(_, _1), EXPAND(FOR_EACH_90(MACRO, _, __VA_ARGS__))
#define FOR_EACH_92(MACRO, _, _1, ...) MACRO(_, _1), EXPAND(FOR_EACH_91(MACRO, _, __VA_ARGS__))
#define FOR_EACH_93(MACRO, _, _1, ...) MACRO(_, _1), EXPAND(FOR_EACH_92(MACRO, _, __VA_ARGS__))
#define FOR_EACH_94(MACRO, _, _1, ...) MACRO(_, _1), EXPAND(FOR_EACH_93(MACRO, _, __VA_ARGS__))
#define FOR_EACH_95(MACRO, _, _1, ...) MACRO(_, _1), EXPAND(FOR_EACH_94(MACRO, _, __VA_ARGS__))
#define FOR_EACH_96(MACRO, _, _1, ...) MACRO(_, _1), EXPAND(FOR_EACH_95(MACRO, _, __VA_ARGS__))
#define FOR_EACH_97(MACRO, _, _1, ...) MACRO(_, _1), EXPAND(FOR_EACH_96(MACRO, _, __VA_ARGS__))
#define FOR_EACH_98(MACRO, _, _1, ...) MACRO(_, _1), EXPAND(FOR_EACH_97(MACRO, _, __VA_ARGS__))
#define FOR_EACH_99(MACRO, _, _1, ...) MACRO(_, _1), EXPAND(FOR_EACH_98(MACRO, _, __VA_ARGS__))
#define FOR_EACH_100(MACRO, _, _1, ...) MACRO(_, _1), EXPAND(FOR_EACH_99(MACRO, _, __VA_ARGS__))
#define FOR_EACH_101(MACRO, _, _1, ...) MACRO(_, _1), EXPAND(FOR_EACH_100(MACRO, _, __VA_ARGS__))
#define FOR_EACH_102(MACRO, _, _1, ...) MACRO(_, _1), EXPAND(FOR_EACH_101(MACRO, _, __VA_ARGS__))
#define FOR_EACH_103(MACRO, _, _1, ...) MACRO(_, _1), EXPAND(FOR_EACH_102(MACRO, _, __VA_ARGS__))
#define FOR_EACH_104(MACRO, _, _1, ...) MACRO(_, _1), EXPAND(FOR_EACH_103(MACRO, _, __VA_ARGS__))
#define FOR_EACH_105(MACRO, _, _1, ...) MACRO(_, _1), EXPAND(FOR_EACH_104(MACRO, _, __VA_ARGS__))
#define FOR_EACH_106(MACRO, _, _1, ...) MACRO(_, _1), EXPAND(FOR_EACH_105(MACRO, _, __VA_ARGS__))
#define FOR_EACH_107(MACRO, _, _1, ...) MACRO(_, _1), EXPAND(FOR_EACH_106(MACRO, _, __VA_ARGS__))
#define FOR_EACH_108(MACRO, _, _1, ...) MACRO(_, _1), EXPAND(FOR_EACH_107(MACRO, _, __VA_ARGS__))
#define FOR_EACH_109(MACRO, _, _1, ...) MACRO(_, _1), EXPAND(FOR_EACH_108(MACRO, _, __VA_ARGS__))
#define FOR_EACH_110(MACRO, _, _1, ...) MACRO(_, _1), EXPAND(FOR_EACH_109(MACRO, _, __VA_ARGS__))
#define FOR_EACH_111(MACRO, _, _1, ...) MACRO(_, _1), EXPAND(FOR_EACH_110(MACRO, _, __VA_ARGS__))
#define FOR_EACH_112(MACRO, _, _1, ...) MACRO(_, _1), EXPAND(FOR_EACH_111(MACRO, _, __VA_ARGS__))
#define FOR_EACH_113(MACRO, _, _1, ...) MACRO(_, _1), EXPAND(FOR_EACH_112(MACRO, _, __VA_ARGS__))
#define FOR_EACH_114(MACRO, _, _1, ...) MACRO(_, _1), EXPAND(FOR_EACH_113(MACRO, _, __VA_ARGS__))
#define FOR_EACH_115(MACRO, _, _1, ...) MACRO(_, _1), EXPAND(FOR_EACH_114(MACRO, _, __VA_ARGS__))
#define FOR_EACH_116(MACRO, _, _1, ...) MACRO(_, _1), EXPAND(FOR_EACH_115(MACRO, _, __VA_ARGS__))
#define FOR_EACH_117(MACRO, _, _1, ...) MACRO(_, _1), EXPAND(FOR_EACH_116(MACRO, _, __VA_ARGS__))
#define FOR_EACH_118(MACRO, _, _1, ...) MACRO(_, _1), EXPAND(FOR_EACH_117(MACRO, _, __VA_ARGS__))
#define FOR_EACH_119(MACRO, _, _1, ...) MACRO(_, _1), EXPAND(FOR_EACH_118(MACRO, _, __VA_ARGS__))
#define FOR_EACH_120(MACRO, _, _1, ...) MACRO(_, _1), EXPAND(FOR_EACH_119(MACRO, _, __VA_ARGS__))
#define FOR_EACH_121(MACRO, _, _1, ...) MACRO(_, _1), EXPAND(FOR_EACH_120(MACRO, _, __VA_ARGS__))
#define FOR_EACH_122(MACRO, _, _1, ...) MACRO(_, _1), EXPAND(FOR_EACH_121(MACRO, _, __VA_ARGS__))
#define FOR_EACH_123(MACRO, _, _1, ...) MACRO(_, _1), EXPAND(FOR_EACH_122(MACRO, _, __VA_ARGS__))
#define FOR_EACH_124(MACRO, _, _1, ...) MACRO(_, _1), EXPAND(FOR_EACH_123(MACRO, _, __VA_ARGS__))
#define FOR_EACH_125(MACRO, _, _1, ...) MACRO(_, _1), EXPAND(FOR_EACH_124(MACRO, _, __VA_ARGS__))
#define FOR_EACH_126(MACRO, _, _1, ...) MACRO(_, _1), EXPAND(FOR_EACH_125(MACRO, _, __VA_ARGS__))
#define FOR_EACH_127(MACRO, _, _1, ...) MACRO(_, _1), EXPAND(FOR_EACH_126(MACRO, _, __VA_ARGS__))
#define FOR_EACH_128(MACRO, _, _1, ...) MACRO(_, _1), EXPAND(FOR_EACH_127(MACRO, _, __VA_ARGS__))
#define FOR_EACH_129(MACRO, _, _1, ...) MACRO(_, _1), EXPAND(FOR_EACH_128(MACRO, _, __VA_ARGS__))
#define FOR_EACH_130(MACRO, _, _1, ...) MACRO(_, _1), EXPAND(FOR_EACH_129(MACRO, _, __VA_ARGS__))
#define FOR_EACH_131(MACRO, _, _1, ...) MACRO(_, _1), EXPAND(FOR_EACH_130(MACRO, _, __VA_ARGS__))
#define FOR_EACH_132(MACRO, _, _1, ...) MACRO(_, _1), EXPAND(FOR_EACH_131(MACRO, _, __VA_ARGS__))
#define FOR_EACH_133(MACRO, _, _1, ...) MACRO(_, _1), EXPAND(FOR_EACH_132(MACRO, _, __VA_ARGS__))
#define FOR_EACH_134(MACRO, _, _1, ...) MACRO(_, _1), EXPAND(FOR_EACH_133(MACRO, _, __VA_ARGS__))
#define FOR_EACH_135(MACRO, _, _1, ...) MACRO(_, _1), EXPAND(FOR_EACH_134(MACRO, _, __VA_ARGS__))
#define FOR_EACH_136(MACRO, _, _1, ...) MACRO(_, _1), EXPAND(FOR_EACH_135(MACRO, _, __VA_ARGS__))
#define FOR_EACH_137(MACRO, _, _1, ...) MACRO(_, _1), EXPAND(FOR_EACH_136(MACRO, _, __VA_ARGS__))
#define FOR_EACH_138(MACRO, _, _1, ...) MACRO(_, _1), EXPAND(FOR_EACH_137(MACRO, _, __VA_ARGS__))
#define FOR_EACH_139(MACRO, _, _1, ...) MACRO(_, _1), EXPAND(FOR_EACH_138(MACRO, _, __VA_ARGS__))
#define FOR_EACH_140(MACRO, _, _1, ...) MACRO(_, _1), EXPAND(FOR_EACH_139(MACRO, _, __VA_ARGS__))
#define FOR_EACH_141(MACRO, _, _1, ...) MACRO(_, _1), EXPAND(FOR_EACH_140(MACRO, _, __VA_ARGS__))
#define FOR_EACH_142(MACRO, _, _1, ...) MACRO(_, _1), EXPAND(FOR_EACH_141(MACRO, _, __VA_ARGS__))
#define FOR_EACH_143(MACRO, _, _1, ...) MACRO(_, _1), EXPAND(FOR_EACH_142(MACRO, _, __VA_ARGS__))
#define FOR_EACH_144(MACRO, _, _1, ...) MACRO(_, _1), EXPAND(FOR_EACH_143(MACRO, _, __VA_ARGS__))
#define FOR_EACH_145(MACRO, _, _1, ...) MACRO(_, _1), EXPAND(FOR_EACH_144(MACRO, _, __VA_ARGS__))
#define FOR_EACH_146(MACRO, _, _1, ...) MACRO(_, _1), EXPAND(FOR_EACH_145(MACRO, _, __VA_ARGS__))
#define FOR_EACH_147(MACRO, _, _1, ...) MACRO(_, _1), EXPAND(FOR_EACH_146(MACRO, _, __VA_ARGS__))
#define FOR_EACH_148(MACRO, _, _1, ...) MACRO(_, _1), EXPAND(FOR_EACH_147(MACRO, _, __VA_ARGS__))
#define FOR_EACH_149(MACRO, _, _1, ...) MACRO(_, _1), EXPAND(FOR_EACH_148(MACRO, _, __VA_ARGS__))
#define FOR_EACH_150(MACRO, _, _1, ...) MACRO(_, _1), EXPAND(FOR_EACH_149(MACRO, _, __VA_ARGS__))
#define FOR_EACH_151(MACRO, _, _1, ...) MACRO(_, _1), EXPAND(FOR_EACH_150(MACRO, _, __VA_ARGS__))
#define FOR_EACH_152(MACRO, _, _1, ...) MACRO(_, _1), EXPAND(FOR_EACH_151(MACRO, _, __VA_ARGS__))
#define FOR_EACH_153(MACRO, _, _1, ...) MACRO(_, _1), EXPAND(FOR_EACH_152(MACRO, _, __VA_ARGS__))
#define FOR_EACH_154(MACRO, _, _1, ...) MACRO(_, _1), EXPAND(FOR_EACH_153(MACRO, _, __VA_ARGS__))
#define FOR_EACH_155(MACRO, _, _1, ...) MACRO(_, _1), EXPAND(FOR_EACH_154(MACRO, _, __VA_ARGS__))
#define FOR_EACH_156(MACRO, _, _1, ...) MACRO(_, _1), EXPAND(FOR_EACH_155(MACRO, _, __VA_ARGS__))
#define FOR_EACH_157(MACRO, _, _1, ...) MACRO(_, _1), EXPAND(FOR_EACH_156(MACRO, _, __VA_ARGS__))
#define FOR_EACH_158(MACRO, _, _1, ...) MACRO(_, _1), EXPAND(FOR_EACH_157(MACRO, _, __VA_ARGS__))
#define FOR_EACH_159(MACRO, _, _1, ...) MACRO(_, _1), EXPAND(FOR_EACH_158(MACRO, _, __VA_ARGS__))
#define FOR_EACH_160(MACRO, _, _1, ...) MACRO(_, _1), EXPAND(FOR_EACH_159(MACRO, _, __VA_ARGS__))
#define FOR_EACH_161(MACRO, _, _1, ...) MACRO(_, _1), EXPAND(FOR_EACH_160(MACRO, _, __VA_ARGS__))
#define FOR_EACH_162(MACRO, _, _1, ...) MACRO(_, _1), EXPAND(FOR_EACH_161(MACRO, _, __VA_ARGS__))
#define FOR_EACH_163(MACRO, _, _1, ...) MACRO(_, _1), EXPAND(FOR_EACH_162(MACRO, _, __VA_ARGS__))
#define FOR_EACH_164(MACRO, _, _1, ...) MACRO(_, _1), EXPAND(FOR_EACH_163(MACRO, _, __VA_ARGS__))
#define FOR_EACH_165(MACRO, _, _1, ...) MACRO(_, _1), EXPAND(FOR_EACH_164(MACRO, _, __VA_ARGS__))
#define FOR_EACH_166(MACRO, _, _1, ...) MACRO(_, _1), EXPAND(FOR_EACH_165(MACRO, _, __VA_ARGS__))
#define FOR_EACH_167(MACRO, _, _1, ...) MACRO(_, _1), EXPAND(FOR_EACH_166(MACRO, _, __VA_ARGS__))
#define FOR_EACH_168(MACRO, _, _1, ...) MACRO(_, _1), EXPAND(FOR_EACH_167(MACRO, _, __VA_ARGS__))
#define FOR_EACH_169(MACRO, _, _1, ...) MACRO(_, _1), EXPAND(FOR_EACH_168(MACRO, _, __VA_ARGS__))
#define FOR_EACH_170(MACRO, _, _1, ...) MACRO(_, _1), EXPAND(FOR_EACH_169(MACRO, _, __VA_ARGS__))
#define FOR_EACH_171(MACRO, _, _1, ...) MACRO(_, _1), EXPAND(FOR_EACH_170(MACRO, _, __VA_ARGS__))
#define FOR_EACH_172(MACRO, _, _1, ...) MACRO(_, _1), EXPAND(FOR_EACH_171(MACRO, _, __VA_ARGS__))
#define FOR_EACH_173(MACRO, _, _1, ...) MACRO(_, _1), EXPAND(FOR_EACH_172(MACRO, _, __VA_ARGS__))
#define FOR_EACH_174(MACRO, _, _1, ...) MACRO(_, _1), EXPAND(FOR_EACH_173(MACRO, _, __VA_ARGS__))
#define FOR_EACH_175(MACRO, _, _1, ...) MACRO(_, _1), EXPAND(FOR_EACH_174(MACRO, _, __VA_ARGS__))
#define FOR_EACH_176(MACRO, _, _1, ...) MACRO(_, _1), EXPAND(FOR_EACH_175(MACRO, _, __VA_ARGS__))
#define FOR_EACH_177(MACRO, _, _1, ...) MACRO(_, _1), EXPAND(FOR_EACH_176(MACRO, _, __VA_ARGS__))
#define FOR_EACH_178(MACRO, _, _1, ...) MACRO(_, _1), EXPAND(FOR_EACH_177(MACRO, _, __VA_ARGS__))
#define FOR_EACH_179(MACRO, _, _1, ...) MACRO(_, _1), EXPAND(FOR_EACH_178(MACRO, _, __VA_ARGS__))
#define FOR_EACH_180(MACRO, _, _1, ...) MACRO(_, _1), EXPAND(FOR_EACH_179(MACRO, _, __VA_ARGS__))
#define FOR_EACH_181(MACRO, _, _1, ...) MACRO(_, _1), EXPAND(FOR_EACH_180(MACRO, _, __VA_ARGS__))
#define FOR_EACH_182(MACRO, _, _1, ...) MACRO(_, _1), EXPAND(FOR_EACH_181(MACRO, _, __VA_ARGS__))
#define FOR_EACH_183(MACRO, _, _1, ...) MACRO(_, _1), EXPAND(FOR_EACH_182(MACRO, _, __VA_ARGS__))
#define FOR_EACH_184(MACRO, _, _1, ...) MACRO(_, _1), EXPAND(FOR_EACH_183(MACRO, _, __VA_ARGS__))
#define FOR_EACH_185(MACRO, _, _1, ...) MACRO(_, _1), EXPAND(FOR_EACH_184(MACRO, _, __VA_ARGS__))
#define FOR_EACH_186(MACRO, _, _1, ...) MACRO(_, _1), EXPAND(FOR_EACH_185(MACRO, _, __VA_ARGS__))
#define FOR_EACH_187(MACRO, _, _1, ...) MACRO(_, _1), EXPAND(FOR_EACH_186(MACRO, _, __VA_ARGS__))
#define FOR_EACH_188(MACRO, _, _1, ...) MACRO(_, _1), EXPAND(FOR_EACH_187(MACRO, _, __VA_ARGS__))
#define FOR_EACH_189(MACRO, _, _1, ...) MACRO(_, _1), EXPAND(FOR_EACH_188(MACRO, _, __VA_ARGS__))
#define FOR_EACH_190(MACRO, _, _1, ...) MACRO(_, _1), EXPAND(FOR_EACH_189(MACRO, _, __VA_ARGS__))
#define FOR_EACH_191(MACRO, _, _1, ...) MACRO(_, _1), EXPAND(FOR_EACH_190(MACRO, _, __VA_ARGS__))
#define FOR_EACH_192(MACRO, _, _1, ...) MACRO(_, _1), EXPAND(FOR_EACH_191(MACRO, _, __VA_ARGS__))
#define FOR_EACH_193(MACRO, _, _1, ...) MACRO(_, _1), EXPAND(FOR_EACH_192(MACRO, _, __VA_ARGS__))
#define FOR_EACH_194(MACRO, _, _1, ...) MACRO(_, _1), EXPAND(FOR_EACH_193(MACRO, _, __VA_ARGS__))
#define FOR_EACH_195(MACRO, _, _1, ...) MACRO(_, _1), EXPAND(FOR_EACH_194(MACRO, _, __VA_ARGS__))
#define FOR_EACH_196(MACRO, _, _1, ...) MACRO(_, _1), EXPAND(FOR_EACH_195(MACRO, _, __VA_ARGS__))
#define FOR_EACH_197(MACRO, _, _1, ...) MACRO(_, _1), EXPAND(FOR_EACH_196(MACRO, _, __VA_ARGS__))
#define FOR_EACH_198(MACRO, _, _1, ...) MACRO(_, _1), EXPAND(FOR_EACH_197(MACRO, _, __VA_ARGS__))
#define FOR_EACH_199(MACRO, _, _1, ...) MACRO(_, _1), EXPAND(FOR_EACH_198(MACRO, _, __VA_ARGS__))
#define FOR_EACH_200(MACRO, _, _1, ...) MACRO(_, _1), EXPAND(FOR_EACH_199(MACRO, _, __VA_ARGS__))
#define FOR_EACH_201(MACRO, _, _1, ...) MACRO(_, _1), EXPAND(FOR_EACH_200(MACRO, _, __VA_ARGS__))
#define FOR_EACH_202(MACRO, _, _1, ...) MACRO(_, _1), EXPAND(FOR_EACH_201(MACRO, _, __VA_ARGS__))
#define FOR_EACH_203(MACRO, _, _1, ...) MACRO(_, _1), EXPAND(FOR_EACH_202(MACRO, _, __VA_ARGS__))
#define FOR_EACH_204(MACRO, _, _1, ...) MACRO(_, _1), EXPAND(FOR_EACH_203(MACRO, _, __VA_ARGS__))
#define FOR_EACH_205(MACRO, _, _1, ...) MACRO(_, _1), EXPAND(FOR_EACH_204(MACRO, _, __VA_ARGS__))
#define FOR_EACH_206(MACRO, _, _1, ...) MACRO(_, _1), EXPAND(FOR_EACH_205(MACRO, _, __VA_ARGS__))
#define FOR_EACH_207(MACRO, _, _1, ...) MACRO(_, _1), EXPAND(FOR_EACH_206(MACRO, _, __VA_ARGS__))
#define FOR_EACH_208(MACRO, _, _1, ...) MACRO(_, _1), EXPAND(FOR_EACH_207(MACRO, _, __VA_ARGS__))
#define FOR_EACH_209(MACRO, _, _1, ...) MACRO(_, _1), EXPAND(FOR_EACH_208(MACRO, _, __VA_ARGS__))
#define FOR_EACH_210(MACRO, _, _1, ...) MACRO(_, _1), EXPAND(FOR_EACH_209(MACRO, _, __VA_ARGS__))
#define FOR_EACH_211(MACRO, _, _1, ...) MACRO(_, _1), EXPAND(FOR_EACH_210(MACRO, _, __VA_ARGS__))
#define FOR_EACH_212(MACRO, _, _1, ...) MACRO(_, _1), EXPAND(FOR_EACH_211(MACRO, _, __VA_ARGS__))
#define FOR_EACH_213(MACRO, _, _1, ...) MACRO(_, _1), EXPAND(FOR_EACH_212(MACRO, _, __VA_ARGS__))
#define FOR_EACH_214(MACRO, _, _1, ...) MACRO(_, _1), EXPAND(FOR_EACH_213(MACRO, _, __VA_ARGS__))
#define FOR_EACH_215(MACRO, _, _1, ...) MACRO(_, _1), EXPAND(FOR_EACH_214(MACRO, _, __VA_ARGS__))
#define FOR_EACH_216(MACRO, _, _1, ...) MACRO(_, _1), EXPAND(FOR_EACH_215(MACRO, _, __VA_ARGS__))
#define FOR_EACH_217(MACRO, _, _1, ...) MACRO(_, _1), EXPAND(FOR_EACH_216(MACRO, _, __VA_ARGS__))
#define FOR_EACH_218(MACRO, _, _1, ...) MACRO(_, _1), EXPAND(FOR_EACH_217(MACRO, _, __VA_ARGS__))
#define FOR_EACH_219(MACRO, _, _1, ...) MACRO(_, _1), EXPAND(FOR_EACH_218(MACRO, _, __VA_ARGS__))
#define FOR_EACH_220(MACRO, _, _1, ...) MACRO(_, _1), EXPAND(FOR_EACH_219(MACRO, _, __VA_ARGS__))
#define FOR_EACH_221(MACRO, _, _1, ...) MACRO(_, _1), EXPAND(FOR_EACH_220(MACRO, _, __VA_ARGS__))
#define FOR_EACH_222(MACRO, _, _1, ...) MACRO(_, _1), EXPAND(FOR_EACH_221(MACRO, _, __VA_ARGS__))
#define FOR_EACH_223(MACRO, _, _1, ...) MACRO(_, _1), EXPAND(FOR_EACH_222(MACRO, _, __VA_ARGS__))
#define FOR_EACH_224(MACRO, _, _1, ...) MACRO(_, _1), EXPAND(FOR_EACH_223(MACRO, _, __VA_ARGS__))
#define FOR_EACH_225(MACRO, _, _1, ...) MACRO(_, _1), EXPAND(FOR_EACH_224(MACRO, _, __VA_ARGS__))
#define FOR_EACH_226(MACRO, _, _1, ...) MACRO(_, _1), EXPAND(FOR_EACH_225(MACRO, _, __VA_ARGS__))
#define FOR_EACH_227(MACRO, _, _1, ...) MACRO(_, _1), EXPAND(FOR_EACH_226(MACRO, _, __VA_ARGS__))
#define FOR_EACH_228(MACRO, _, _1, ...) MACRO(_, _1), EXPAND(FOR_EACH_227(MACRO, _, __VA_ARGS__))
#define FOR_EACH_229(MACRO, _, _1, ...) MACRO(_, _1), EXPAND(FOR_EACH_228(MACRO, _, __VA_ARGS__))
#define FOR_EACH_230(MACRO, _, _1, ...) MACRO(_, _1), EXPAND(FOR_EACH_229(MACRO, _, __VA_ARGS__))
#define FOR_EACH_231(MACRO, _, _1, ...) MACRO(_, _1), EXPAND(FOR_EACH_230(MACRO, _, __VA_ARGS__))
#define FOR_EACH_232(MACRO, _, _1, ...) MACRO(_, _1), EXPAND(FOR_EACH_231(MACRO, _, __VA_ARGS__))
#define FOR_EACH_233(MACRO, _, _1, ...) MACRO(_, _1), EXPAND(FOR_EACH_232(MACRO, _, __VA_ARGS__))
#define FOR_EACH_234(MACRO, _, _1, ...) MACRO(_, _1), EXPAND(FOR_EACH_233(MACRO, _, __VA_ARGS__))
#define FOR_EACH_235(MACRO, _, _1, ...) MACRO(_, _1), EXPAND(FOR_EACH_234(MACRO, _, __VA_ARGS__))
#define FOR_EACH_236(MACRO, _, _1, ...) MACRO(_, _1), EXPAND(FOR_EACH_235(MACRO, _, __VA_ARGS__))
#define FOR_EACH_237(MACRO, _, _1, ...) MACRO(_, _1), EXPAND(FOR_EACH_236(MACRO, _, __VA_ARGS__))
#define FOR_EACH_238(MACRO, _, _1, ...) MACRO(_, _1), EXPAND(FOR_EACH_237(MACRO, _, __VA_ARGS__))
#define FOR_EACH_239(MACRO, _, _1, ...) MACRO(_, _1), EXPAND(FOR_EACH_238(MACRO, _, __VA_ARGS__))
#define FOR_EACH_240(MACRO, _, _1, ...) MACRO(_, _1), EXPAND(FOR_EACH_239(MACRO, _, __VA_ARGS__))
#define FOR_EACH_241(MACRO, _, _1, ...) MACRO(_, _1), EXPAND(FOR_EACH_240(MACRO, _, __VA_ARGS__))
#define FOR_EACH_242(MACRO, _, _1, ...) MACRO(_, _1), EXPAND(FOR_EACH_241(MACRO, _, __VA_ARGS__))
#define FOR_EACH_243(MACRO, _, _1, ...) MACRO(_, _1), EXPAND(FOR_EACH_242(MACRO, _, __VA_ARGS__))
#define FOR_EACH_244(MACRO, _, _1, ...) MACRO(_, _1), EXPAND(FOR_EACH_243(MACRO, _, __VA_ARGS__))
#define FOR_EACH_245(MACRO, _, _1, ...) MACRO(_, _1), EXPAND(FOR_EACH_244(MACRO, _, __VA_ARGS__))
#define FOR_EACH_246(MACRO, _, _1, ...) MACRO(_, _1), EXPAND(FOR_EACH_245(MACRO, _, __VA_ARGS__))
#define FOR_EACH_247(MACRO, _, _1, ...) MACRO(_, _1), EXPAND(FOR_EACH_246(MACRO, _, __VA_ARGS__))
#define FOR_EACH_248(MACRO, _, _1, ...) MACRO(_, _1), EXPAND(FOR_EACH_247(MACRO, _, __VA_ARGS__))
#define FOR_EACH_249(MACRO, _, _1, ...) MACRO(_, _1), EXPAND(FOR_EACH_248(MACRO, _, __VA_ARGS__))
#define FOR_EACH_250(MACRO, _, _1, ...) MACRO(_, _1), EXPAND(FOR_EACH_249(MACRO, _, __VA_ARGS__))
#define FOR_EACH_251(MACRO, _, _1, ...) MACRO(_, _1), EXPAND(FOR_EACH_250(MACRO, _, __VA_ARGS__))
#define FOR_EACH_252(MACRO, _, _1, ...) MACRO(_, _1), EXPAND(FOR_EACH_251(MACRO, _, __VA_ARGS__))
#define FOR_EACH_253(MACRO, _, _1, ...) MACRO(_, _1), EXPAND(FOR_EACH_252(MACRO, _, __VA_ARGS__))
#define FOR_EACH_254(MACRO, _, _1, ...) MACRO(_, _1), EXPAND(FOR_EACH_253(MACRO, _, __VA_ARGS__))
#define FOR_EACH_255(MACRO, _, _1, ...) MACRO(_, _1), EXPAND(FOR_EACH_254(MACRO, _, __VA_ARGS__))

#define FOR_EACH__(MACRO, _, ...) CONCAT(FOR_EACH_, NUM_ARGS(__VA_ARGS__))(MACRO, _, ##__VA_ARGS__)
#define FOR_EACH(MACRO, _, ...) FOR_EACH__(MACRO, _, ##__VA_ARGS__)

#define REFL_NAME(_, name)                                                                         \
    template <>                                                                                    \
    struct ::atom::utils::nickname<_> {                                                            \
        constexpr static inline std::string_view value = #name;                                    \
    }

#define REFL_MEMBERS(_, ...)                                                                       \
    constexpr static inline auto field_traits() noexcept {                                         \
        return std::make_tuple(FOR_EACH(FIELD_TRAITS, _, ##__VA_ARGS__));                          \
    } //

#define REFL_FUNCS(_, ...)                                                                         \
    constexpr static inline auto function_traits() noexcept {                                      \
        return std::make_tuple(FOR_EACH(FUNCTION_TRAITS, _, ##__VA_ARGS__));                       \
    } //
