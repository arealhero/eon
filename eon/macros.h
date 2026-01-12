#pragma once

#define CONCATENATE(lhs, rhs)   CONCATENATE_2(lhs, rhs)
#define CONCATENATE_2(lhs, rhs) CONCATENATE_1(lhs, rhs)
#define CONCATENATE_1(lhs, rhs) lhs ## rhs

// @ref: NUMBER_OF_ARGS implementation if based on https://groups.google.com/g/comp.std.c/c/d-6Mj5Lko_s
#define GET_NTH_ARGUMENT(                               \
    _49, _48, _47, _46, _45, _44, _43, _42, _41, _40,   \
    _39, _38, _37, _36, _35, _34, _33, _32, _31, _30,   \
    _29, _28, _27, _26, _25, _24, _23, _22, _21, _20,   \
    _19, _18, _17, _16, _15, _14, _13, _12, _11, _10,   \
    _9, _8, _7, _6, _5, _4, _3, _2, _1, N, ...)         \
    N
#define REVERSED_INDEX_SEQUENCE                 \
    49, 48, 47, 46, 45, 44, 43, 42, 41, 40,     \
    39, 38, 37, 36, 35, 34, 33, 32, 31, 30,     \
    29, 28, 27, 26, 25, 24, 23, 22, 21, 20,     \
    19, 18, 17, 16, 15, 14, 13, 12, 11, 10,     \
    9, 8, 7, 6, 5, 4, 3, 2, 1, 0

#define NUMBER_OF_ARGS_IMPL(...) GET_NTH_ARGUMENT(__VA_ARGS__)

// FIXME(vlad): Test if __VA_OPT__ is available and use hand-made comma detection if it is not.
//              @ref: https://t6847kimo.github.io/blog/2019/02/04/Remove-comma-in-variadic-macro.html
#define NUMBER_OF_ARGS(...)                                             \
    NUMBER_OF_ARGS_IMPL(__VA_ARGS__ __VA_OPT__(,) REVERSED_INDEX_SEQUENCE)

#define FOR_EACH_0(...)
#define FOR_EACH_1(DO, x)       DO(x)
#define FOR_EACH_2(DO, x, ...)  DO(x) FOR_EACH_1(DO, __VA_ARGS__)
#define FOR_EACH_3(DO, x, ...)  DO(x) FOR_EACH_2(DO, __VA_ARGS__)
#define FOR_EACH_4(DO, x, ...)  DO(x) FOR_EACH_3(DO, __VA_ARGS__)
#define FOR_EACH_5(DO, x, ...)  DO(x) FOR_EACH_4(DO, __VA_ARGS__)
#define FOR_EACH_6(DO, x, ...)  DO(x) FOR_EACH_5(DO, __VA_ARGS__)
#define FOR_EACH_7(DO, x, ...)  DO(x) FOR_EACH_6(DO, __VA_ARGS__)
#define FOR_EACH_8(DO, x, ...)  DO(x) FOR_EACH_7(DO, __VA_ARGS__)
#define FOR_EACH_9(DO, x, ...)  DO(x) FOR_EACH_8(DO, __VA_ARGS__)
#define FOR_EACH_10(DO, x, ...) DO(x) FOR_EACH_09(DO, __VA_ARGS__)
#define FOR_EACH_11(DO, x, ...) DO(x) FOR_EACH_10(DO, __VA_ARGS__)
#define FOR_EACH_12(DO, x, ...) DO(x) FOR_EACH_11(DO, __VA_ARGS__)
#define FOR_EACH_13(DO, x, ...) DO(x) FOR_EACH_12(DO, __VA_ARGS__)
#define FOR_EACH_14(DO, x, ...) DO(x) FOR_EACH_13(DO, __VA_ARGS__)
#define FOR_EACH_15(DO, x, ...) DO(x) FOR_EACH_14(DO, __VA_ARGS__)
#define FOR_EACH_16(DO, x, ...) DO(x) FOR_EACH_15(DO, __VA_ARGS__)
#define FOR_EACH_17(DO, x, ...) DO(x) FOR_EACH_16(DO, __VA_ARGS__)
#define FOR_EACH_18(DO, x, ...) DO(x) FOR_EACH_17(DO, __VA_ARGS__)
#define FOR_EACH_19(DO, x, ...) DO(x) FOR_EACH_18(DO, __VA_ARGS__)
#define FOR_EACH_20(DO, x, ...) DO(x) FOR_EACH_19(DO, __VA_ARGS__)
#define FOR_EACH_21(DO, x, ...) DO(x) FOR_EACH_20(DO, __VA_ARGS__)
#define FOR_EACH_22(DO, x, ...) DO(x) FOR_EACH_21(DO, __VA_ARGS__)
#define FOR_EACH_23(DO, x, ...) DO(x) FOR_EACH_22(DO, __VA_ARGS__)
#define FOR_EACH_24(DO, x, ...) DO(x) FOR_EACH_23(DO, __VA_ARGS__)
#define FOR_EACH_25(DO, x, ...) DO(x) FOR_EACH_24(DO, __VA_ARGS__)
#define FOR_EACH_26(DO, x, ...) DO(x) FOR_EACH_25(DO, __VA_ARGS__)
#define FOR_EACH_27(DO, x, ...) DO(x) FOR_EACH_26(DO, __VA_ARGS__)
#define FOR_EACH_28(DO, x, ...) DO(x) FOR_EACH_27(DO, __VA_ARGS__)
#define FOR_EACH_29(DO, x, ...) DO(x) FOR_EACH_28(DO, __VA_ARGS__)
#define FOR_EACH_30(DO, x, ...) DO(x) FOR_EACH_29(DO, __VA_ARGS__)
#define FOR_EACH_31(DO, x, ...) DO(x) FOR_EACH_30(DO, __VA_ARGS__)
#define FOR_EACH_32(DO, x, ...) DO(x) FOR_EACH_31(DO, __VA_ARGS__)
#define FOR_EACH_33(DO, x, ...) DO(x) FOR_EACH_32(DO, __VA_ARGS__)
#define FOR_EACH_34(DO, x, ...) DO(x) FOR_EACH_33(DO, __VA_ARGS__)
#define FOR_EACH_35(DO, x, ...) DO(x) FOR_EACH_34(DO, __VA_ARGS__)
#define FOR_EACH_36(DO, x, ...) DO(x) FOR_EACH_35(DO, __VA_ARGS__)
#define FOR_EACH_37(DO, x, ...) DO(x) FOR_EACH_36(DO, __VA_ARGS__)
#define FOR_EACH_38(DO, x, ...) DO(x) FOR_EACH_37(DO, __VA_ARGS__)
#define FOR_EACH_39(DO, x, ...) DO(x) FOR_EACH_38(DO, __VA_ARGS__)
#define FOR_EACH_40(DO, x, ...) DO(x) FOR_EACH_39(DO, __VA_ARGS__)
#define FOR_EACH_41(DO, x, ...) DO(x) FOR_EACH_40(DO, __VA_ARGS__)
#define FOR_EACH_42(DO, x, ...) DO(x) FOR_EACH_41(DO, __VA_ARGS__)
#define FOR_EACH_43(DO, x, ...) DO(x) FOR_EACH_42(DO, __VA_ARGS__)
#define FOR_EACH_44(DO, x, ...) DO(x) FOR_EACH_43(DO, __VA_ARGS__)
#define FOR_EACH_45(DO, x, ...) DO(x) FOR_EACH_44(DO, __VA_ARGS__)
#define FOR_EACH_46(DO, x, ...) DO(x) FOR_EACH_45(DO, __VA_ARGS__)
#define FOR_EACH_47(DO, x, ...) DO(x) FOR_EACH_46(DO, __VA_ARGS__)

#define FOR_EACH_IMPL(N, DO, ...) CONCATENATE(FOR_EACH_, N)(DO, __VA_ARGS__)
#define FOR_EACH(DO, ...) FOR_EACH_IMPL(NUMBER_OF_ARGS(__VA_ARGS__), DO, __VA_ARGS__)

#define REQUIRE_SEMICOLON void CONCATENATE(Semicolon_Required_, __COUNTER__)(void)
