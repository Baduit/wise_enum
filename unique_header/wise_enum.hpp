#pragma once

#include <array>
#include <type_traits>
#include <utility>

// optional type needed for interface
#ifndef WISE_ENUM_OPTIONAL_TYPE
#if __cplusplus == 201703L
#include <optional>
namespace wise_enum {
template <class T>
using optional_type = std::optional<T>;
}
#else

#include <stdexcept>
#include <type_traits>

#if __cplusplus == 201103
#define WISE_ENUM_CONSTEXPR_14
#else
#define WISE_ENUM_CONSTEXPR_14 constexpr
#endif

namespace wise_enum {

struct bad_optional_access : std::exception {
  const char *what() const noexcept override {
    return "Error, attempt to access valueless optional!";
  }
};

/* A simple, *forward* compatible optional implementation. That is, it does not
 * provide the full std::optional interface, but all the interface it does
 * provide is found on std::optional, so it should not be a breaking change to
 * upgrade to std::optional.
 */
template <class T>
class optional {
public:
  static_assert(std::is_enum<T>::value,
                "wise enum optional is only for enum types");
  optional() = default;
  optional(T t) : m_t(t), m_active(true) {}

  WISE_ENUM_CONSTEXPR_14 T &operator*() & { return m_t; }
  constexpr const T &operator*() const & { return m_t; }
  WISE_ENUM_CONSTEXPR_14 T &&operator*() && { return m_t; }
  constexpr const T &&operator*() const && { return m_t; }

  constexpr explicit operator bool() const noexcept { return m_active; }
  constexpr bool has_value() const noexcept { return m_active; }

  WISE_ENUM_CONSTEXPR_14 T &value() & {
    if (m_active)
      return m_t;
    else
      throw bad_optional_access{};
  }
  constexpr const T &value() const & {
    if (m_active)
      return m_t;
    else
      throw bad_optional_access{};
  }

  WISE_ENUM_CONSTEXPR_14 T &&value() && {
    if (m_active)
      return m_t;
    else
      throw bad_optional_access{};
  }
  constexpr const T &&value() const && {
    if (m_active)
      return m_t;
    else
      throw bad_optional_access{};
  }

  template <class U>
  constexpr T value_or(U &&u) {
    if (m_active)
      return m_t;
    else
      return std::forward<U>(u);
  }

  void reset() noexcept { m_active = false; }

  optional(const optional &other) = default;
  optional(optional &&other) = default;
  optional &operator=(const optional &other) = default;
  optional &operator=(optional &&other) = default;

private:
  T m_t;
  bool m_active = false;
};
} // namespace wise_enum
namespace wise_enum {
template <class T>
using optional_type = wise_enum::optional<T>;
}
#endif
#else
namespace wise_enum {
template <class T>
using optional_type = WISE_ENUM_OPTIONAL_TYPE<T>;
}
#endif

// Choice of string_view if type defined, otherwise use string literal
#ifndef WISE_ENUM_STRING_TYPE
#if __cplusplus == 201703L
#include <string_view>
namespace wise_enum {
using string_type = std::string_view;
}
#else
namespace wise_enum {
using string_type = const char *;
}
#endif
#else
namespace wise_enum {
using string_type = WISE_ENUM_STRING_TYPE;
}
#endif

#if __cplusplus == 201103
#define WISE_ENUM_CONSTEXPR_14
#else
#define WISE_ENUM_CONSTEXPR_14 constexpr
#endif

namespace wise_enum {
namespace detail {

template <class T>
struct value_and_name {
  T value;
  string_type name;
};

template <class T>
struct Tag {};

constexpr void wise_enum_detail_array(...);

template <class T>
struct is_wise_enum
    : std::integral_constant<
          bool, !std::is_same<void, decltype(wise_enum_detail_array(
                                        Tag<T>{}))>::value> {};

WISE_ENUM_CONSTEXPR_14 int strcmp(const char *s1, const char *s2) {
  while (*s1 && (*s1 == *s2))
    s1++, s2++;
  if (*s1 < *s2) {
    return -1;
  }
  if (*s1 > *s2) {
    return 1;
  } else {
    return 0;
  }
}

WISE_ENUM_CONSTEXPR_14 bool compare(const char *s1, const char *s2) {
  return strcmp(s1, s2) == 0;
}

template <class U, class = typename std::enable_if<
                       !std::is_same<U, const char *>::value>::type>
WISE_ENUM_CONSTEXPR_14 bool compare(U u1, U u2) {
  return u1 == u2;
}
} // namespace detail
} // namespace wise_enum

#define WISE_ENUM_IMPL_NARG(...)                                               \
  WISE_ENUM_IMPL_NARG_(__VA_ARGS__, WISE_ENUM_IMPL_RSEQ_N())
#define WISE_ENUM_IMPL_NARG_(...) WISE_ENUM_IMPL_ARG_N(__VA_ARGS__)

// ARG_N and RSEQ_N defined in wise_enum_generated.h

// Building blocks; credit to:
// https://github.com/pfultz2/Cloak/wiki/C-Preprocessor-tricks,-tips,-and-idioms

#define WISE_ENUM_IMPL_COMMA() ,
#define WISE_ENUM_IMPL_NOTHING()

#define WISE_ENUM_IMPL_CAT(a, ...) WISE_ENUM_IMPL_PRIMITIVE_CAT(a, __VA_ARGS__)
#define WISE_ENUM_IMPL_PRIMITIVE_CAT(a, ...) a##__VA_ARGS__

#define WISE_ENUM_IMPL_XSTR(s) #s
#define WISE_ENUM_IMPL_STR(s) WISE_ENUM_IMPL_XSTR(s)

#define WISE_ENUM_IMPL_IIF(c) WISE_ENUM_IMPL_CAT(WISE_ENUM_IMPL_IIF_, c)
#define WISE_ENUM_IMPL_IIF_0(t, f) f
#define WISE_ENUM_IMPL_IIF_1(t, f) t

#define WISE_ENUM_IMPL_CHECK_N(x, n, ...) n
#define WISE_ENUM_IMPL_CHECK(...) WISE_ENUM_IMPL_CHECK_N(__VA_ARGS__, 0, )
#define WISE_ENUM_IMPL_PROBE(x) x, 1,

#define WISE_ENUM_IMPL_IS_PAREN(x)                                             \
  WISE_ENUM_IMPL_CHECK(WISE_ENUM_IMPL_IS_PAREN_PROBE x)
#define WISE_ENUM_IMPL_IS_PAREN_PROBE(...) WISE_ENUM_IMPL_PROBE(~)

#define WISE_ENUM_IMPL_FIRST_ARG(x, ...) x

#define WISE_ENUM_IMPL_ONLY_OR_FIRST(x)                                        \
  WISE_ENUM_IMPL_IIF(WISE_ENUM_IMPL_IS_PAREN(x))                               \
  (WISE_ENUM_IMPL_FIRST_ARG x, x)

// Use building blocks to conditionally process enumerators; they can either be
// just an identifier, or (identifier, value)
#define WISE_ENUM_IMPL_ENUM_INIT_2(x, ...) x = __VA_ARGS__
#define WISE_ENUM_IMPL_ENUM_INIT(name, x)                                      \
  WISE_ENUM_IMPL_IIF(WISE_ENUM_IMPL_IS_PAREN(x))                               \
  (WISE_ENUM_IMPL_ENUM_INIT_2 x, x)

#define WISE_ENUM_IMPL_ENUM_STR(x)                                             \
  WISE_ENUM_IMPL_STR(WISE_ENUM_IMPL_ONLY_OR_FIRST(x))

#define WISE_ENUM_IMPL_DESC_PAIR(name, x)                                      \
  { name::WISE_ENUM_IMPL_ONLY_OR_FIRST(x), WISE_ENUM_IMPL_ENUM_STR(x) }

#define WISE_ENUM_IMPL_SWITCH_CASE(name, x)                                    \
  case name::WISE_ENUM_IMPL_ONLY_OR_FIRST(x):                                  \
    return WISE_ENUM_IMPL_ENUM_STR(x);

#define WISE_ENUM_IMPL_STORAGE_2(x, y) y

#define WISE_ENUM_IMPL_STORAGE(x)                                              \
  WISE_ENUM_IMPL_IIF(WISE_ENUM_IMPL_IS_PAREN(x))                               \
  ( : WISE_ENUM_IMPL_STORAGE_2 x, )

#define WISE_ENUM_IMPL(type, name_storage, friendly, ...)                      \
  WISE_ENUM_IMPL_2(type, WISE_ENUM_IMPL_ONLY_OR_FIRST(name_storage),           \
                   WISE_ENUM_IMPL_STORAGE(name_storage), friendly,             \
                   WISE_ENUM_IMPL_NARG(__VA_ARGS__), __VA_ARGS__)

#define WISE_ENUM_IMPL_2(type, name, storage, friendly, num_enums, ...)        \
  WISE_ENUM_IMPL_3(type, name, storage, friendly, num_enums,                   \
                   WISE_ENUM_IMPL_CAT(WISE_ENUM_IMPL_LOOP_, num_enums),        \
                   __VA_ARGS__)

#define WISE_ENUM_IMPL_3(type, name, storage, friendly, num_enums, loop, ...)  \
  type name storage{                                                           \
      loop(WISE_ENUM_IMPL_ENUM_INIT, _, WISE_ENUM_IMPL_COMMA, __VA_ARGS__)};   \
  WISE_ENUM_IMPL_ADAPT_3(name, friendly, num_enums, loop, __VA_ARGS__)

#define WISE_ENUM_IMPL_ADAPT(name, ...)                                        \
  namespace wise_enum {                                                        \
  namespace detail {                                                           \
  WISE_ENUM_IMPL_ADAPT_2(name, WISE_ENUM_IMPL_NARG(__VA_ARGS__), __VA_ARGS__)  \
  }                                                                            \
  }

#define WISE_ENUM_IMPL_ADAPT_2(name, num_enums, ...)                           \
  WISE_ENUM_IMPL_ADAPT_3(name, , num_enums,                                    \
                         WISE_ENUM_IMPL_CAT(WISE_ENUM_IMPL_LOOP_, num_enums),  \
                         __VA_ARGS__)

#define WISE_ENUM_IMPL_ADAPT_3(name, friendly, num_enums, loop, ...)           \
  friendly constexpr std::array<::wise_enum::detail::value_and_name<name>,     \
                                num_enums>                                     \
  wise_enum_detail_array(::wise_enum::detail::Tag<name>) {                     \
    return {{loop(WISE_ENUM_IMPL_DESC_PAIR, name, WISE_ENUM_IMPL_COMMA,        \
                  __VA_ARGS__)}};                                              \
  }                                                                            \
                                                                               \
  template <class T>                                                           \
  friendly WISE_ENUM_CONSTEXPR_14 ::wise_enum::string_type                     \
  wise_enum_detail_to_string(T e, ::wise_enum::detail::Tag<name>) {            \
    switch (e) {                                                               \
      loop(WISE_ENUM_IMPL_SWITCH_CASE, name, WISE_ENUM_IMPL_NOTHING,           \
           __VA_ARGS__)                                                        \
    }                                                                          \
    return {};                                                                 \
  }
#define WISE_ENUM_IMPL_ARG_N(                                                           \
    _1, _2, _3, _4, _5, _6, _7, _8, _9, _10,                                            \
    _11, _12, _13, _14, _15, _16, _17, _18, _19, _20,                                   \
    _21, _22, _23, _24, _25, _26, _27, _28, _29, _30,                                   \
    _31, _32, _33, _34, _35, _36, _37, _38, _39, _40,                                   \
    _41, _42, _43, _44, _45, _46, _47, _48, _49, _50,                                   \
    _51, _52, _53, _54, _55, _56, _57, _58, _59, _60,                                   \
    _61, _62, _63, _64, _65, _66, _67, _68, _69, _70,                                   \
    _71, _72, _73, _74, _75, _76, _77, _78, _79, _80,                                   \
    _81, _82, _83, _84, _85, _86, _87, _88, _89, _90,                                   \
    _91, _92, _93, _94, _95, _96, _97, _98, _99, _100,                                  \
    _101, _102, _103, _104, _105, _106, _107, _108, _109, _110,                         \
    _111, _112, _113, _114, _115, _116, _117, _118, _119, _120,                         \
    _121, _122, _123, _124, _125, _126, _127, _128, _129, _130,                         \
    _131, _132, _133, _134, _135, _136, _137, _138, _139, _140,                         \
    _141, _142, _143, _144, _145, _146, _147, _148, _149, _150,                         \
    _151, _152, _153, _154, _155, _156, _157, _158, _159, _160,                         \
    _161, _162, _163, _164, _165, _166, _167, _168, _169, _170,                         \
    _171, _172, _173, _174, _175, _176, _177, _178, _179, _180,                         \
    _181, _182, _183, _184, _185, _186, _187, _188, _189, _190,                         \
    _191, _192, _193, _194, _195, _196, _197, _198, _199, _200,                         \
    _201, _202, _203, _204, _205, _206, _207, _208, _209, _210,                         \
    _211, _212, _213, _214, _215, _216, _217, _218, _219, _220,                         \
    _221, _222, _223, _224, _225, _226, _227, _228, _229, _230,                         \
    _231, _232, _233, _234, _235, _236, _237, _238, _239, _240,                         \
    _241, _242, _243, _244, _245, _246, _247, _248, _249, _250,                         \
    _251, _252, _253, _254, _255, _256, N, ...                                          \
  )                                                                                     \
  N                                                                                     \

#define WISE_ENUM_IMPL_RSEQ_N()                                                         \
  256, 255, 254, 253, 252, 251, 250, 249, 248, 247,                                     \
  246, 245, 244, 243, 242, 241, 240, 239, 238, 237,                                     \
  236, 235, 234, 233, 232, 231, 230, 229, 228, 227,                                     \
  226, 225, 224, 223, 222, 221, 220, 219, 218, 217,                                     \
  216, 215, 214, 213, 212, 211, 210, 209, 208, 207,                                     \
  206, 205, 204, 203, 202, 201, 200, 199, 198, 197,                                     \
  196, 195, 194, 193, 192, 191, 190, 189, 188, 187,                                     \
  186, 185, 184, 183, 182, 181, 180, 179, 178, 177,                                     \
  176, 175, 174, 173, 172, 171, 170, 169, 168, 167,                                     \
  166, 165, 164, 163, 162, 161, 160, 159, 158, 157,                                     \
  156, 155, 154, 153, 152, 151, 150, 149, 148, 147,                                     \
  146, 145, 144, 143, 142, 141, 140, 139, 138, 137,                                     \
  136, 135, 134, 133, 132, 131, 130, 129, 128, 127,                                     \
  126, 125, 124, 123, 122, 121, 120, 119, 118, 117,                                     \
  116, 115, 114, 113, 112, 111, 110, 109, 108, 107,                                     \
  106, 105, 104, 103, 102, 101, 100, 99, 98, 97,                                        \
  96, 95, 94, 93, 92, 91, 90, 89, 88, 87,                                               \
  86, 85, 84, 83, 82, 81, 80, 79, 78, 77,                                               \
  76, 75, 74, 73, 72, 71, 70, 69, 68, 67,                                               \
  66, 65, 64, 63, 62, 61, 60, 59, 58, 57,                                               \
  56, 55, 54, 53, 52, 51, 50, 49, 48, 47,                                               \
  46, 45, 44, 43, 42, 41, 40, 39, 38, 37,                                               \
  36, 35, 34, 33, 32, 31, 30, 29, 28, 27,                                               \
  26, 25, 24, 23, 22, 21, 20, 19, 18, 17,                                               \
  16, 15, 14, 13, 12, 11, 10, 9, 8, 7,                                                  \
  6, 5, 4, 3, 2, 1, 0                                                                   \

#define WISE_ENUM_IMPL_LOOP_1(M, C, D, x) M(C, x)

#define WISE_ENUM_IMPL_LOOP_2(M, C, D, x, ...) M(C, x) D()                              \
  WISE_ENUM_IMPL_LOOP_1(M, C, D, __VA_ARGS__)

#define WISE_ENUM_IMPL_LOOP_3(M, C, D, x, ...) M(C, x) D()                              \
  WISE_ENUM_IMPL_LOOP_2(M, C, D, __VA_ARGS__)

#define WISE_ENUM_IMPL_LOOP_4(M, C, D, x, ...) M(C, x) D()                              \
  WISE_ENUM_IMPL_LOOP_3(M, C, D, __VA_ARGS__)

#define WISE_ENUM_IMPL_LOOP_5(M, C, D, x, ...) M(C, x) D()                              \
  WISE_ENUM_IMPL_LOOP_4(M, C, D, __VA_ARGS__)

#define WISE_ENUM_IMPL_LOOP_6(M, C, D, x, ...) M(C, x) D()                              \
  WISE_ENUM_IMPL_LOOP_5(M, C, D, __VA_ARGS__)

#define WISE_ENUM_IMPL_LOOP_7(M, C, D, x, ...) M(C, x) D()                              \
  WISE_ENUM_IMPL_LOOP_6(M, C, D, __VA_ARGS__)

#define WISE_ENUM_IMPL_LOOP_8(M, C, D, x, ...) M(C, x) D()                              \
  WISE_ENUM_IMPL_LOOP_7(M, C, D, __VA_ARGS__)

#define WISE_ENUM_IMPL_LOOP_9(M, C, D, x, ...) M(C, x) D()                              \
  WISE_ENUM_IMPL_LOOP_8(M, C, D, __VA_ARGS__)

#define WISE_ENUM_IMPL_LOOP_10(M, C, D, x, ...) M(C, x) D()                             \
  WISE_ENUM_IMPL_LOOP_9(M, C, D, __VA_ARGS__)

#define WISE_ENUM_IMPL_LOOP_11(M, C, D, x, ...) M(C, x) D()                             \
  WISE_ENUM_IMPL_LOOP_10(M, C, D, __VA_ARGS__)

#define WISE_ENUM_IMPL_LOOP_12(M, C, D, x, ...) M(C, x) D()                             \
  WISE_ENUM_IMPL_LOOP_11(M, C, D, __VA_ARGS__)

#define WISE_ENUM_IMPL_LOOP_13(M, C, D, x, ...) M(C, x) D()                             \
  WISE_ENUM_IMPL_LOOP_12(M, C, D, __VA_ARGS__)

#define WISE_ENUM_IMPL_LOOP_14(M, C, D, x, ...) M(C, x) D()                             \
  WISE_ENUM_IMPL_LOOP_13(M, C, D, __VA_ARGS__)

#define WISE_ENUM_IMPL_LOOP_15(M, C, D, x, ...) M(C, x) D()                             \
  WISE_ENUM_IMPL_LOOP_14(M, C, D, __VA_ARGS__)

#define WISE_ENUM_IMPL_LOOP_16(M, C, D, x, ...) M(C, x) D()                             \
  WISE_ENUM_IMPL_LOOP_15(M, C, D, __VA_ARGS__)

#define WISE_ENUM_IMPL_LOOP_17(M, C, D, x, ...) M(C, x) D()                             \
  WISE_ENUM_IMPL_LOOP_16(M, C, D, __VA_ARGS__)

#define WISE_ENUM_IMPL_LOOP_18(M, C, D, x, ...) M(C, x) D()                             \
  WISE_ENUM_IMPL_LOOP_17(M, C, D, __VA_ARGS__)

#define WISE_ENUM_IMPL_LOOP_19(M, C, D, x, ...) M(C, x) D()                             \
  WISE_ENUM_IMPL_LOOP_18(M, C, D, __VA_ARGS__)

#define WISE_ENUM_IMPL_LOOP_20(M, C, D, x, ...) M(C, x) D()                             \
  WISE_ENUM_IMPL_LOOP_19(M, C, D, __VA_ARGS__)

#define WISE_ENUM_IMPL_LOOP_21(M, C, D, x, ...) M(C, x) D()                             \
  WISE_ENUM_IMPL_LOOP_20(M, C, D, __VA_ARGS__)

#define WISE_ENUM_IMPL_LOOP_22(M, C, D, x, ...) M(C, x) D()                             \
  WISE_ENUM_IMPL_LOOP_21(M, C, D, __VA_ARGS__)

#define WISE_ENUM_IMPL_LOOP_23(M, C, D, x, ...) M(C, x) D()                             \
  WISE_ENUM_IMPL_LOOP_22(M, C, D, __VA_ARGS__)

#define WISE_ENUM_IMPL_LOOP_24(M, C, D, x, ...) M(C, x) D()                             \
  WISE_ENUM_IMPL_LOOP_23(M, C, D, __VA_ARGS__)

#define WISE_ENUM_IMPL_LOOP_25(M, C, D, x, ...) M(C, x) D()                             \
  WISE_ENUM_IMPL_LOOP_24(M, C, D, __VA_ARGS__)

#define WISE_ENUM_IMPL_LOOP_26(M, C, D, x, ...) M(C, x) D()                             \
  WISE_ENUM_IMPL_LOOP_25(M, C, D, __VA_ARGS__)

#define WISE_ENUM_IMPL_LOOP_27(M, C, D, x, ...) M(C, x) D()                             \
  WISE_ENUM_IMPL_LOOP_26(M, C, D, __VA_ARGS__)

#define WISE_ENUM_IMPL_LOOP_28(M, C, D, x, ...) M(C, x) D()                             \
  WISE_ENUM_IMPL_LOOP_27(M, C, D, __VA_ARGS__)

#define WISE_ENUM_IMPL_LOOP_29(M, C, D, x, ...) M(C, x) D()                             \
  WISE_ENUM_IMPL_LOOP_28(M, C, D, __VA_ARGS__)

#define WISE_ENUM_IMPL_LOOP_30(M, C, D, x, ...) M(C, x) D()                             \
  WISE_ENUM_IMPL_LOOP_29(M, C, D, __VA_ARGS__)

#define WISE_ENUM_IMPL_LOOP_31(M, C, D, x, ...) M(C, x) D()                             \
  WISE_ENUM_IMPL_LOOP_30(M, C, D, __VA_ARGS__)

#define WISE_ENUM_IMPL_LOOP_32(M, C, D, x, ...) M(C, x) D()                             \
  WISE_ENUM_IMPL_LOOP_31(M, C, D, __VA_ARGS__)

#define WISE_ENUM_IMPL_LOOP_33(M, C, D, x, ...) M(C, x) D()                             \
  WISE_ENUM_IMPL_LOOP_32(M, C, D, __VA_ARGS__)

#define WISE_ENUM_IMPL_LOOP_34(M, C, D, x, ...) M(C, x) D()                             \
  WISE_ENUM_IMPL_LOOP_33(M, C, D, __VA_ARGS__)

#define WISE_ENUM_IMPL_LOOP_35(M, C, D, x, ...) M(C, x) D()                             \
  WISE_ENUM_IMPL_LOOP_34(M, C, D, __VA_ARGS__)

#define WISE_ENUM_IMPL_LOOP_36(M, C, D, x, ...) M(C, x) D()                             \
  WISE_ENUM_IMPL_LOOP_35(M, C, D, __VA_ARGS__)

#define WISE_ENUM_IMPL_LOOP_37(M, C, D, x, ...) M(C, x) D()                             \
  WISE_ENUM_IMPL_LOOP_36(M, C, D, __VA_ARGS__)

#define WISE_ENUM_IMPL_LOOP_38(M, C, D, x, ...) M(C, x) D()                             \
  WISE_ENUM_IMPL_LOOP_37(M, C, D, __VA_ARGS__)

#define WISE_ENUM_IMPL_LOOP_39(M, C, D, x, ...) M(C, x) D()                             \
  WISE_ENUM_IMPL_LOOP_38(M, C, D, __VA_ARGS__)

#define WISE_ENUM_IMPL_LOOP_40(M, C, D, x, ...) M(C, x) D()                             \
  WISE_ENUM_IMPL_LOOP_39(M, C, D, __VA_ARGS__)

#define WISE_ENUM_IMPL_LOOP_41(M, C, D, x, ...) M(C, x) D()                             \
  WISE_ENUM_IMPL_LOOP_40(M, C, D, __VA_ARGS__)

#define WISE_ENUM_IMPL_LOOP_42(M, C, D, x, ...) M(C, x) D()                             \
  WISE_ENUM_IMPL_LOOP_41(M, C, D, __VA_ARGS__)

#define WISE_ENUM_IMPL_LOOP_43(M, C, D, x, ...) M(C, x) D()                             \
  WISE_ENUM_IMPL_LOOP_42(M, C, D, __VA_ARGS__)

#define WISE_ENUM_IMPL_LOOP_44(M, C, D, x, ...) M(C, x) D()                             \
  WISE_ENUM_IMPL_LOOP_43(M, C, D, __VA_ARGS__)

#define WISE_ENUM_IMPL_LOOP_45(M, C, D, x, ...) M(C, x) D()                             \
  WISE_ENUM_IMPL_LOOP_44(M, C, D, __VA_ARGS__)

#define WISE_ENUM_IMPL_LOOP_46(M, C, D, x, ...) M(C, x) D()                             \
  WISE_ENUM_IMPL_LOOP_45(M, C, D, __VA_ARGS__)

#define WISE_ENUM_IMPL_LOOP_47(M, C, D, x, ...) M(C, x) D()                             \
  WISE_ENUM_IMPL_LOOP_46(M, C, D, __VA_ARGS__)

#define WISE_ENUM_IMPL_LOOP_48(M, C, D, x, ...) M(C, x) D()                             \
  WISE_ENUM_IMPL_LOOP_47(M, C, D, __VA_ARGS__)

#define WISE_ENUM_IMPL_LOOP_49(M, C, D, x, ...) M(C, x) D()                             \
  WISE_ENUM_IMPL_LOOP_48(M, C, D, __VA_ARGS__)

#define WISE_ENUM_IMPL_LOOP_50(M, C, D, x, ...) M(C, x) D()                             \
  WISE_ENUM_IMPL_LOOP_49(M, C, D, __VA_ARGS__)

#define WISE_ENUM_IMPL_LOOP_51(M, C, D, x, ...) M(C, x) D()                             \
  WISE_ENUM_IMPL_LOOP_50(M, C, D, __VA_ARGS__)

#define WISE_ENUM_IMPL_LOOP_52(M, C, D, x, ...) M(C, x) D()                             \
  WISE_ENUM_IMPL_LOOP_51(M, C, D, __VA_ARGS__)

#define WISE_ENUM_IMPL_LOOP_53(M, C, D, x, ...) M(C, x) D()                             \
  WISE_ENUM_IMPL_LOOP_52(M, C, D, __VA_ARGS__)

#define WISE_ENUM_IMPL_LOOP_54(M, C, D, x, ...) M(C, x) D()                             \
  WISE_ENUM_IMPL_LOOP_53(M, C, D, __VA_ARGS__)

#define WISE_ENUM_IMPL_LOOP_55(M, C, D, x, ...) M(C, x) D()                             \
  WISE_ENUM_IMPL_LOOP_54(M, C, D, __VA_ARGS__)

#define WISE_ENUM_IMPL_LOOP_56(M, C, D, x, ...) M(C, x) D()                             \
  WISE_ENUM_IMPL_LOOP_55(M, C, D, __VA_ARGS__)

#define WISE_ENUM_IMPL_LOOP_57(M, C, D, x, ...) M(C, x) D()                             \
  WISE_ENUM_IMPL_LOOP_56(M, C, D, __VA_ARGS__)

#define WISE_ENUM_IMPL_LOOP_58(M, C, D, x, ...) M(C, x) D()                             \
  WISE_ENUM_IMPL_LOOP_57(M, C, D, __VA_ARGS__)

#define WISE_ENUM_IMPL_LOOP_59(M, C, D, x, ...) M(C, x) D()                             \
  WISE_ENUM_IMPL_LOOP_58(M, C, D, __VA_ARGS__)

#define WISE_ENUM_IMPL_LOOP_60(M, C, D, x, ...) M(C, x) D()                             \
  WISE_ENUM_IMPL_LOOP_59(M, C, D, __VA_ARGS__)

#define WISE_ENUM_IMPL_LOOP_61(M, C, D, x, ...) M(C, x) D()                             \
  WISE_ENUM_IMPL_LOOP_60(M, C, D, __VA_ARGS__)

#define WISE_ENUM_IMPL_LOOP_62(M, C, D, x, ...) M(C, x) D()                             \
  WISE_ENUM_IMPL_LOOP_61(M, C, D, __VA_ARGS__)

#define WISE_ENUM_IMPL_LOOP_63(M, C, D, x, ...) M(C, x) D()                             \
  WISE_ENUM_IMPL_LOOP_62(M, C, D, __VA_ARGS__)

#define WISE_ENUM_IMPL_LOOP_64(M, C, D, x, ...) M(C, x) D()                             \
  WISE_ENUM_IMPL_LOOP_63(M, C, D, __VA_ARGS__)

#define WISE_ENUM_IMPL_LOOP_65(M, C, D, x, ...) M(C, x) D()                             \
  WISE_ENUM_IMPL_LOOP_64(M, C, D, __VA_ARGS__)

#define WISE_ENUM_IMPL_LOOP_66(M, C, D, x, ...) M(C, x) D()                             \
  WISE_ENUM_IMPL_LOOP_65(M, C, D, __VA_ARGS__)

#define WISE_ENUM_IMPL_LOOP_67(M, C, D, x, ...) M(C, x) D()                             \
  WISE_ENUM_IMPL_LOOP_66(M, C, D, __VA_ARGS__)

#define WISE_ENUM_IMPL_LOOP_68(M, C, D, x, ...) M(C, x) D()                             \
  WISE_ENUM_IMPL_LOOP_67(M, C, D, __VA_ARGS__)

#define WISE_ENUM_IMPL_LOOP_69(M, C, D, x, ...) M(C, x) D()                             \
  WISE_ENUM_IMPL_LOOP_68(M, C, D, __VA_ARGS__)

#define WISE_ENUM_IMPL_LOOP_70(M, C, D, x, ...) M(C, x) D()                             \
  WISE_ENUM_IMPL_LOOP_69(M, C, D, __VA_ARGS__)

#define WISE_ENUM_IMPL_LOOP_71(M, C, D, x, ...) M(C, x) D()                             \
  WISE_ENUM_IMPL_LOOP_70(M, C, D, __VA_ARGS__)

#define WISE_ENUM_IMPL_LOOP_72(M, C, D, x, ...) M(C, x) D()                             \
  WISE_ENUM_IMPL_LOOP_71(M, C, D, __VA_ARGS__)

#define WISE_ENUM_IMPL_LOOP_73(M, C, D, x, ...) M(C, x) D()                             \
  WISE_ENUM_IMPL_LOOP_72(M, C, D, __VA_ARGS__)

#define WISE_ENUM_IMPL_LOOP_74(M, C, D, x, ...) M(C, x) D()                             \
  WISE_ENUM_IMPL_LOOP_73(M, C, D, __VA_ARGS__)

#define WISE_ENUM_IMPL_LOOP_75(M, C, D, x, ...) M(C, x) D()                             \
  WISE_ENUM_IMPL_LOOP_74(M, C, D, __VA_ARGS__)

#define WISE_ENUM_IMPL_LOOP_76(M, C, D, x, ...) M(C, x) D()                             \
  WISE_ENUM_IMPL_LOOP_75(M, C, D, __VA_ARGS__)

#define WISE_ENUM_IMPL_LOOP_77(M, C, D, x, ...) M(C, x) D()                             \
  WISE_ENUM_IMPL_LOOP_76(M, C, D, __VA_ARGS__)

#define WISE_ENUM_IMPL_LOOP_78(M, C, D, x, ...) M(C, x) D()                             \
  WISE_ENUM_IMPL_LOOP_77(M, C, D, __VA_ARGS__)

#define WISE_ENUM_IMPL_LOOP_79(M, C, D, x, ...) M(C, x) D()                             \
  WISE_ENUM_IMPL_LOOP_78(M, C, D, __VA_ARGS__)

#define WISE_ENUM_IMPL_LOOP_80(M, C, D, x, ...) M(C, x) D()                             \
  WISE_ENUM_IMPL_LOOP_79(M, C, D, __VA_ARGS__)

#define WISE_ENUM_IMPL_LOOP_81(M, C, D, x, ...) M(C, x) D()                             \
  WISE_ENUM_IMPL_LOOP_80(M, C, D, __VA_ARGS__)

#define WISE_ENUM_IMPL_LOOP_82(M, C, D, x, ...) M(C, x) D()                             \
  WISE_ENUM_IMPL_LOOP_81(M, C, D, __VA_ARGS__)

#define WISE_ENUM_IMPL_LOOP_83(M, C, D, x, ...) M(C, x) D()                             \
  WISE_ENUM_IMPL_LOOP_82(M, C, D, __VA_ARGS__)

#define WISE_ENUM_IMPL_LOOP_84(M, C, D, x, ...) M(C, x) D()                             \
  WISE_ENUM_IMPL_LOOP_83(M, C, D, __VA_ARGS__)

#define WISE_ENUM_IMPL_LOOP_85(M, C, D, x, ...) M(C, x) D()                             \
  WISE_ENUM_IMPL_LOOP_84(M, C, D, __VA_ARGS__)

#define WISE_ENUM_IMPL_LOOP_86(M, C, D, x, ...) M(C, x) D()                             \
  WISE_ENUM_IMPL_LOOP_85(M, C, D, __VA_ARGS__)

#define WISE_ENUM_IMPL_LOOP_87(M, C, D, x, ...) M(C, x) D()                             \
  WISE_ENUM_IMPL_LOOP_86(M, C, D, __VA_ARGS__)

#define WISE_ENUM_IMPL_LOOP_88(M, C, D, x, ...) M(C, x) D()                             \
  WISE_ENUM_IMPL_LOOP_87(M, C, D, __VA_ARGS__)

#define WISE_ENUM_IMPL_LOOP_89(M, C, D, x, ...) M(C, x) D()                             \
  WISE_ENUM_IMPL_LOOP_88(M, C, D, __VA_ARGS__)

#define WISE_ENUM_IMPL_LOOP_90(M, C, D, x, ...) M(C, x) D()                             \
  WISE_ENUM_IMPL_LOOP_89(M, C, D, __VA_ARGS__)

#define WISE_ENUM_IMPL_LOOP_91(M, C, D, x, ...) M(C, x) D()                             \
  WISE_ENUM_IMPL_LOOP_90(M, C, D, __VA_ARGS__)

#define WISE_ENUM_IMPL_LOOP_92(M, C, D, x, ...) M(C, x) D()                             \
  WISE_ENUM_IMPL_LOOP_91(M, C, D, __VA_ARGS__)

#define WISE_ENUM_IMPL_LOOP_93(M, C, D, x, ...) M(C, x) D()                             \
  WISE_ENUM_IMPL_LOOP_92(M, C, D, __VA_ARGS__)

#define WISE_ENUM_IMPL_LOOP_94(M, C, D, x, ...) M(C, x) D()                             \
  WISE_ENUM_IMPL_LOOP_93(M, C, D, __VA_ARGS__)

#define WISE_ENUM_IMPL_LOOP_95(M, C, D, x, ...) M(C, x) D()                             \
  WISE_ENUM_IMPL_LOOP_94(M, C, D, __VA_ARGS__)

#define WISE_ENUM_IMPL_LOOP_96(M, C, D, x, ...) M(C, x) D()                             \
  WISE_ENUM_IMPL_LOOP_95(M, C, D, __VA_ARGS__)

#define WISE_ENUM_IMPL_LOOP_97(M, C, D, x, ...) M(C, x) D()                             \
  WISE_ENUM_IMPL_LOOP_96(M, C, D, __VA_ARGS__)

#define WISE_ENUM_IMPL_LOOP_98(M, C, D, x, ...) M(C, x) D()                             \
  WISE_ENUM_IMPL_LOOP_97(M, C, D, __VA_ARGS__)

#define WISE_ENUM_IMPL_LOOP_99(M, C, D, x, ...) M(C, x) D()                             \
  WISE_ENUM_IMPL_LOOP_98(M, C, D, __VA_ARGS__)

#define WISE_ENUM_IMPL_LOOP_100(M, C, D, x, ...) M(C, x) D()                            \
  WISE_ENUM_IMPL_LOOP_99(M, C, D, __VA_ARGS__)

#define WISE_ENUM_IMPL_LOOP_101(M, C, D, x, ...) M(C, x) D()                            \
  WISE_ENUM_IMPL_LOOP_100(M, C, D, __VA_ARGS__)

#define WISE_ENUM_IMPL_LOOP_102(M, C, D, x, ...) M(C, x) D()                            \
  WISE_ENUM_IMPL_LOOP_101(M, C, D, __VA_ARGS__)

#define WISE_ENUM_IMPL_LOOP_103(M, C, D, x, ...) M(C, x) D()                            \
  WISE_ENUM_IMPL_LOOP_102(M, C, D, __VA_ARGS__)

#define WISE_ENUM_IMPL_LOOP_104(M, C, D, x, ...) M(C, x) D()                            \
  WISE_ENUM_IMPL_LOOP_103(M, C, D, __VA_ARGS__)

#define WISE_ENUM_IMPL_LOOP_105(M, C, D, x, ...) M(C, x) D()                            \
  WISE_ENUM_IMPL_LOOP_104(M, C, D, __VA_ARGS__)

#define WISE_ENUM_IMPL_LOOP_106(M, C, D, x, ...) M(C, x) D()                            \
  WISE_ENUM_IMPL_LOOP_105(M, C, D, __VA_ARGS__)

#define WISE_ENUM_IMPL_LOOP_107(M, C, D, x, ...) M(C, x) D()                            \
  WISE_ENUM_IMPL_LOOP_106(M, C, D, __VA_ARGS__)

#define WISE_ENUM_IMPL_LOOP_108(M, C, D, x, ...) M(C, x) D()                            \
  WISE_ENUM_IMPL_LOOP_107(M, C, D, __VA_ARGS__)

#define WISE_ENUM_IMPL_LOOP_109(M, C, D, x, ...) M(C, x) D()                            \
  WISE_ENUM_IMPL_LOOP_108(M, C, D, __VA_ARGS__)

#define WISE_ENUM_IMPL_LOOP_110(M, C, D, x, ...) M(C, x) D()                            \
  WISE_ENUM_IMPL_LOOP_109(M, C, D, __VA_ARGS__)

#define WISE_ENUM_IMPL_LOOP_111(M, C, D, x, ...) M(C, x) D()                            \
  WISE_ENUM_IMPL_LOOP_110(M, C, D, __VA_ARGS__)

#define WISE_ENUM_IMPL_LOOP_112(M, C, D, x, ...) M(C, x) D()                            \
  WISE_ENUM_IMPL_LOOP_111(M, C, D, __VA_ARGS__)

#define WISE_ENUM_IMPL_LOOP_113(M, C, D, x, ...) M(C, x) D()                            \
  WISE_ENUM_IMPL_LOOP_112(M, C, D, __VA_ARGS__)

#define WISE_ENUM_IMPL_LOOP_114(M, C, D, x, ...) M(C, x) D()                            \
  WISE_ENUM_IMPL_LOOP_113(M, C, D, __VA_ARGS__)

#define WISE_ENUM_IMPL_LOOP_115(M, C, D, x, ...) M(C, x) D()                            \
  WISE_ENUM_IMPL_LOOP_114(M, C, D, __VA_ARGS__)

#define WISE_ENUM_IMPL_LOOP_116(M, C, D, x, ...) M(C, x) D()                            \
  WISE_ENUM_IMPL_LOOP_115(M, C, D, __VA_ARGS__)

#define WISE_ENUM_IMPL_LOOP_117(M, C, D, x, ...) M(C, x) D()                            \
  WISE_ENUM_IMPL_LOOP_116(M, C, D, __VA_ARGS__)

#define WISE_ENUM_IMPL_LOOP_118(M, C, D, x, ...) M(C, x) D()                            \
  WISE_ENUM_IMPL_LOOP_117(M, C, D, __VA_ARGS__)

#define WISE_ENUM_IMPL_LOOP_119(M, C, D, x, ...) M(C, x) D()                            \
  WISE_ENUM_IMPL_LOOP_118(M, C, D, __VA_ARGS__)

#define WISE_ENUM_IMPL_LOOP_120(M, C, D, x, ...) M(C, x) D()                            \
  WISE_ENUM_IMPL_LOOP_119(M, C, D, __VA_ARGS__)

#define WISE_ENUM_IMPL_LOOP_121(M, C, D, x, ...) M(C, x) D()                            \
  WISE_ENUM_IMPL_LOOP_120(M, C, D, __VA_ARGS__)

#define WISE_ENUM_IMPL_LOOP_122(M, C, D, x, ...) M(C, x) D()                            \
  WISE_ENUM_IMPL_LOOP_121(M, C, D, __VA_ARGS__)

#define WISE_ENUM_IMPL_LOOP_123(M, C, D, x, ...) M(C, x) D()                            \
  WISE_ENUM_IMPL_LOOP_122(M, C, D, __VA_ARGS__)

#define WISE_ENUM_IMPL_LOOP_124(M, C, D, x, ...) M(C, x) D()                            \
  WISE_ENUM_IMPL_LOOP_123(M, C, D, __VA_ARGS__)

#define WISE_ENUM_IMPL_LOOP_125(M, C, D, x, ...) M(C, x) D()                            \
  WISE_ENUM_IMPL_LOOP_124(M, C, D, __VA_ARGS__)

#define WISE_ENUM_IMPL_LOOP_126(M, C, D, x, ...) M(C, x) D()                            \
  WISE_ENUM_IMPL_LOOP_125(M, C, D, __VA_ARGS__)

#define WISE_ENUM_IMPL_LOOP_127(M, C, D, x, ...) M(C, x) D()                            \
  WISE_ENUM_IMPL_LOOP_126(M, C, D, __VA_ARGS__)

#define WISE_ENUM_IMPL_LOOP_128(M, C, D, x, ...) M(C, x) D()                            \
  WISE_ENUM_IMPL_LOOP_127(M, C, D, __VA_ARGS__)

#define WISE_ENUM_IMPL_LOOP_129(M, C, D, x, ...) M(C, x) D()                            \
  WISE_ENUM_IMPL_LOOP_128(M, C, D, __VA_ARGS__)

#define WISE_ENUM_IMPL_LOOP_130(M, C, D, x, ...) M(C, x) D()                            \
  WISE_ENUM_IMPL_LOOP_129(M, C, D, __VA_ARGS__)

#define WISE_ENUM_IMPL_LOOP_131(M, C, D, x, ...) M(C, x) D()                            \
  WISE_ENUM_IMPL_LOOP_130(M, C, D, __VA_ARGS__)

#define WISE_ENUM_IMPL_LOOP_132(M, C, D, x, ...) M(C, x) D()                            \
  WISE_ENUM_IMPL_LOOP_131(M, C, D, __VA_ARGS__)

#define WISE_ENUM_IMPL_LOOP_133(M, C, D, x, ...) M(C, x) D()                            \
  WISE_ENUM_IMPL_LOOP_132(M, C, D, __VA_ARGS__)

#define WISE_ENUM_IMPL_LOOP_134(M, C, D, x, ...) M(C, x) D()                            \
  WISE_ENUM_IMPL_LOOP_133(M, C, D, __VA_ARGS__)

#define WISE_ENUM_IMPL_LOOP_135(M, C, D, x, ...) M(C, x) D()                            \
  WISE_ENUM_IMPL_LOOP_134(M, C, D, __VA_ARGS__)

#define WISE_ENUM_IMPL_LOOP_136(M, C, D, x, ...) M(C, x) D()                            \
  WISE_ENUM_IMPL_LOOP_135(M, C, D, __VA_ARGS__)

#define WISE_ENUM_IMPL_LOOP_137(M, C, D, x, ...) M(C, x) D()                            \
  WISE_ENUM_IMPL_LOOP_136(M, C, D, __VA_ARGS__)

#define WISE_ENUM_IMPL_LOOP_138(M, C, D, x, ...) M(C, x) D()                            \
  WISE_ENUM_IMPL_LOOP_137(M, C, D, __VA_ARGS__)

#define WISE_ENUM_IMPL_LOOP_139(M, C, D, x, ...) M(C, x) D()                            \
  WISE_ENUM_IMPL_LOOP_138(M, C, D, __VA_ARGS__)

#define WISE_ENUM_IMPL_LOOP_140(M, C, D, x, ...) M(C, x) D()                            \
  WISE_ENUM_IMPL_LOOP_139(M, C, D, __VA_ARGS__)

#define WISE_ENUM_IMPL_LOOP_141(M, C, D, x, ...) M(C, x) D()                            \
  WISE_ENUM_IMPL_LOOP_140(M, C, D, __VA_ARGS__)

#define WISE_ENUM_IMPL_LOOP_142(M, C, D, x, ...) M(C, x) D()                            \
  WISE_ENUM_IMPL_LOOP_141(M, C, D, __VA_ARGS__)

#define WISE_ENUM_IMPL_LOOP_143(M, C, D, x, ...) M(C, x) D()                            \
  WISE_ENUM_IMPL_LOOP_142(M, C, D, __VA_ARGS__)

#define WISE_ENUM_IMPL_LOOP_144(M, C, D, x, ...) M(C, x) D()                            \
  WISE_ENUM_IMPL_LOOP_143(M, C, D, __VA_ARGS__)

#define WISE_ENUM_IMPL_LOOP_145(M, C, D, x, ...) M(C, x) D()                            \
  WISE_ENUM_IMPL_LOOP_144(M, C, D, __VA_ARGS__)

#define WISE_ENUM_IMPL_LOOP_146(M, C, D, x, ...) M(C, x) D()                            \
  WISE_ENUM_IMPL_LOOP_145(M, C, D, __VA_ARGS__)

#define WISE_ENUM_IMPL_LOOP_147(M, C, D, x, ...) M(C, x) D()                            \
  WISE_ENUM_IMPL_LOOP_146(M, C, D, __VA_ARGS__)

#define WISE_ENUM_IMPL_LOOP_148(M, C, D, x, ...) M(C, x) D()                            \
  WISE_ENUM_IMPL_LOOP_147(M, C, D, __VA_ARGS__)

#define WISE_ENUM_IMPL_LOOP_149(M, C, D, x, ...) M(C, x) D()                            \
  WISE_ENUM_IMPL_LOOP_148(M, C, D, __VA_ARGS__)

#define WISE_ENUM_IMPL_LOOP_150(M, C, D, x, ...) M(C, x) D()                            \
  WISE_ENUM_IMPL_LOOP_149(M, C, D, __VA_ARGS__)

#define WISE_ENUM_IMPL_LOOP_151(M, C, D, x, ...) M(C, x) D()                            \
  WISE_ENUM_IMPL_LOOP_150(M, C, D, __VA_ARGS__)

#define WISE_ENUM_IMPL_LOOP_152(M, C, D, x, ...) M(C, x) D()                            \
  WISE_ENUM_IMPL_LOOP_151(M, C, D, __VA_ARGS__)

#define WISE_ENUM_IMPL_LOOP_153(M, C, D, x, ...) M(C, x) D()                            \
  WISE_ENUM_IMPL_LOOP_152(M, C, D, __VA_ARGS__)

#define WISE_ENUM_IMPL_LOOP_154(M, C, D, x, ...) M(C, x) D()                            \
  WISE_ENUM_IMPL_LOOP_153(M, C, D, __VA_ARGS__)

#define WISE_ENUM_IMPL_LOOP_155(M, C, D, x, ...) M(C, x) D()                            \
  WISE_ENUM_IMPL_LOOP_154(M, C, D, __VA_ARGS__)

#define WISE_ENUM_IMPL_LOOP_156(M, C, D, x, ...) M(C, x) D()                            \
  WISE_ENUM_IMPL_LOOP_155(M, C, D, __VA_ARGS__)

#define WISE_ENUM_IMPL_LOOP_157(M, C, D, x, ...) M(C, x) D()                            \
  WISE_ENUM_IMPL_LOOP_156(M, C, D, __VA_ARGS__)

#define WISE_ENUM_IMPL_LOOP_158(M, C, D, x, ...) M(C, x) D()                            \
  WISE_ENUM_IMPL_LOOP_157(M, C, D, __VA_ARGS__)

#define WISE_ENUM_IMPL_LOOP_159(M, C, D, x, ...) M(C, x) D()                            \
  WISE_ENUM_IMPL_LOOP_158(M, C, D, __VA_ARGS__)

#define WISE_ENUM_IMPL_LOOP_160(M, C, D, x, ...) M(C, x) D()                            \
  WISE_ENUM_IMPL_LOOP_159(M, C, D, __VA_ARGS__)

#define WISE_ENUM_IMPL_LOOP_161(M, C, D, x, ...) M(C, x) D()                            \
  WISE_ENUM_IMPL_LOOP_160(M, C, D, __VA_ARGS__)

#define WISE_ENUM_IMPL_LOOP_162(M, C, D, x, ...) M(C, x) D()                            \
  WISE_ENUM_IMPL_LOOP_161(M, C, D, __VA_ARGS__)

#define WISE_ENUM_IMPL_LOOP_163(M, C, D, x, ...) M(C, x) D()                            \
  WISE_ENUM_IMPL_LOOP_162(M, C, D, __VA_ARGS__)

#define WISE_ENUM_IMPL_LOOP_164(M, C, D, x, ...) M(C, x) D()                            \
  WISE_ENUM_IMPL_LOOP_163(M, C, D, __VA_ARGS__)

#define WISE_ENUM_IMPL_LOOP_165(M, C, D, x, ...) M(C, x) D()                            \
  WISE_ENUM_IMPL_LOOP_164(M, C, D, __VA_ARGS__)

#define WISE_ENUM_IMPL_LOOP_166(M, C, D, x, ...) M(C, x) D()                            \
  WISE_ENUM_IMPL_LOOP_165(M, C, D, __VA_ARGS__)

#define WISE_ENUM_IMPL_LOOP_167(M, C, D, x, ...) M(C, x) D()                            \
  WISE_ENUM_IMPL_LOOP_166(M, C, D, __VA_ARGS__)

#define WISE_ENUM_IMPL_LOOP_168(M, C, D, x, ...) M(C, x) D()                            \
  WISE_ENUM_IMPL_LOOP_167(M, C, D, __VA_ARGS__)

#define WISE_ENUM_IMPL_LOOP_169(M, C, D, x, ...) M(C, x) D()                            \
  WISE_ENUM_IMPL_LOOP_168(M, C, D, __VA_ARGS__)

#define WISE_ENUM_IMPL_LOOP_170(M, C, D, x, ...) M(C, x) D()                            \
  WISE_ENUM_IMPL_LOOP_169(M, C, D, __VA_ARGS__)

#define WISE_ENUM_IMPL_LOOP_171(M, C, D, x, ...) M(C, x) D()                            \
  WISE_ENUM_IMPL_LOOP_170(M, C, D, __VA_ARGS__)

#define WISE_ENUM_IMPL_LOOP_172(M, C, D, x, ...) M(C, x) D()                            \
  WISE_ENUM_IMPL_LOOP_171(M, C, D, __VA_ARGS__)

#define WISE_ENUM_IMPL_LOOP_173(M, C, D, x, ...) M(C, x) D()                            \
  WISE_ENUM_IMPL_LOOP_172(M, C, D, __VA_ARGS__)

#define WISE_ENUM_IMPL_LOOP_174(M, C, D, x, ...) M(C, x) D()                            \
  WISE_ENUM_IMPL_LOOP_173(M, C, D, __VA_ARGS__)

#define WISE_ENUM_IMPL_LOOP_175(M, C, D, x, ...) M(C, x) D()                            \
  WISE_ENUM_IMPL_LOOP_174(M, C, D, __VA_ARGS__)

#define WISE_ENUM_IMPL_LOOP_176(M, C, D, x, ...) M(C, x) D()                            \
  WISE_ENUM_IMPL_LOOP_175(M, C, D, __VA_ARGS__)

#define WISE_ENUM_IMPL_LOOP_177(M, C, D, x, ...) M(C, x) D()                            \
  WISE_ENUM_IMPL_LOOP_176(M, C, D, __VA_ARGS__)

#define WISE_ENUM_IMPL_LOOP_178(M, C, D, x, ...) M(C, x) D()                            \
  WISE_ENUM_IMPL_LOOP_177(M, C, D, __VA_ARGS__)

#define WISE_ENUM_IMPL_LOOP_179(M, C, D, x, ...) M(C, x) D()                            \
  WISE_ENUM_IMPL_LOOP_178(M, C, D, __VA_ARGS__)

#define WISE_ENUM_IMPL_LOOP_180(M, C, D, x, ...) M(C, x) D()                            \
  WISE_ENUM_IMPL_LOOP_179(M, C, D, __VA_ARGS__)

#define WISE_ENUM_IMPL_LOOP_181(M, C, D, x, ...) M(C, x) D()                            \
  WISE_ENUM_IMPL_LOOP_180(M, C, D, __VA_ARGS__)

#define WISE_ENUM_IMPL_LOOP_182(M, C, D, x, ...) M(C, x) D()                            \
  WISE_ENUM_IMPL_LOOP_181(M, C, D, __VA_ARGS__)

#define WISE_ENUM_IMPL_LOOP_183(M, C, D, x, ...) M(C, x) D()                            \
  WISE_ENUM_IMPL_LOOP_182(M, C, D, __VA_ARGS__)

#define WISE_ENUM_IMPL_LOOP_184(M, C, D, x, ...) M(C, x) D()                            \
  WISE_ENUM_IMPL_LOOP_183(M, C, D, __VA_ARGS__)

#define WISE_ENUM_IMPL_LOOP_185(M, C, D, x, ...) M(C, x) D()                            \
  WISE_ENUM_IMPL_LOOP_184(M, C, D, __VA_ARGS__)

#define WISE_ENUM_IMPL_LOOP_186(M, C, D, x, ...) M(C, x) D()                            \
  WISE_ENUM_IMPL_LOOP_185(M, C, D, __VA_ARGS__)

#define WISE_ENUM_IMPL_LOOP_187(M, C, D, x, ...) M(C, x) D()                            \
  WISE_ENUM_IMPL_LOOP_186(M, C, D, __VA_ARGS__)

#define WISE_ENUM_IMPL_LOOP_188(M, C, D, x, ...) M(C, x) D()                            \
  WISE_ENUM_IMPL_LOOP_187(M, C, D, __VA_ARGS__)

#define WISE_ENUM_IMPL_LOOP_189(M, C, D, x, ...) M(C, x) D()                            \
  WISE_ENUM_IMPL_LOOP_188(M, C, D, __VA_ARGS__)

#define WISE_ENUM_IMPL_LOOP_190(M, C, D, x, ...) M(C, x) D()                            \
  WISE_ENUM_IMPL_LOOP_189(M, C, D, __VA_ARGS__)

#define WISE_ENUM_IMPL_LOOP_191(M, C, D, x, ...) M(C, x) D()                            \
  WISE_ENUM_IMPL_LOOP_190(M, C, D, __VA_ARGS__)

#define WISE_ENUM_IMPL_LOOP_192(M, C, D, x, ...) M(C, x) D()                            \
  WISE_ENUM_IMPL_LOOP_191(M, C, D, __VA_ARGS__)

#define WISE_ENUM_IMPL_LOOP_193(M, C, D, x, ...) M(C, x) D()                            \
  WISE_ENUM_IMPL_LOOP_192(M, C, D, __VA_ARGS__)

#define WISE_ENUM_IMPL_LOOP_194(M, C, D, x, ...) M(C, x) D()                            \
  WISE_ENUM_IMPL_LOOP_193(M, C, D, __VA_ARGS__)

#define WISE_ENUM_IMPL_LOOP_195(M, C, D, x, ...) M(C, x) D()                            \
  WISE_ENUM_IMPL_LOOP_194(M, C, D, __VA_ARGS__)

#define WISE_ENUM_IMPL_LOOP_196(M, C, D, x, ...) M(C, x) D()                            \
  WISE_ENUM_IMPL_LOOP_195(M, C, D, __VA_ARGS__)

#define WISE_ENUM_IMPL_LOOP_197(M, C, D, x, ...) M(C, x) D()                            \
  WISE_ENUM_IMPL_LOOP_196(M, C, D, __VA_ARGS__)

#define WISE_ENUM_IMPL_LOOP_198(M, C, D, x, ...) M(C, x) D()                            \
  WISE_ENUM_IMPL_LOOP_197(M, C, D, __VA_ARGS__)

#define WISE_ENUM_IMPL_LOOP_199(M, C, D, x, ...) M(C, x) D()                            \
  WISE_ENUM_IMPL_LOOP_198(M, C, D, __VA_ARGS__)

#define WISE_ENUM_IMPL_LOOP_200(M, C, D, x, ...) M(C, x) D()                            \
  WISE_ENUM_IMPL_LOOP_199(M, C, D, __VA_ARGS__)

#define WISE_ENUM_IMPL_LOOP_201(M, C, D, x, ...) M(C, x) D()                            \
  WISE_ENUM_IMPL_LOOP_200(M, C, D, __VA_ARGS__)

#define WISE_ENUM_IMPL_LOOP_202(M, C, D, x, ...) M(C, x) D()                            \
  WISE_ENUM_IMPL_LOOP_201(M, C, D, __VA_ARGS__)

#define WISE_ENUM_IMPL_LOOP_203(M, C, D, x, ...) M(C, x) D()                            \
  WISE_ENUM_IMPL_LOOP_202(M, C, D, __VA_ARGS__)

#define WISE_ENUM_IMPL_LOOP_204(M, C, D, x, ...) M(C, x) D()                            \
  WISE_ENUM_IMPL_LOOP_203(M, C, D, __VA_ARGS__)

#define WISE_ENUM_IMPL_LOOP_205(M, C, D, x, ...) M(C, x) D()                            \
  WISE_ENUM_IMPL_LOOP_204(M, C, D, __VA_ARGS__)

#define WISE_ENUM_IMPL_LOOP_206(M, C, D, x, ...) M(C, x) D()                            \
  WISE_ENUM_IMPL_LOOP_205(M, C, D, __VA_ARGS__)

#define WISE_ENUM_IMPL_LOOP_207(M, C, D, x, ...) M(C, x) D()                            \
  WISE_ENUM_IMPL_LOOP_206(M, C, D, __VA_ARGS__)

#define WISE_ENUM_IMPL_LOOP_208(M, C, D, x, ...) M(C, x) D()                            \
  WISE_ENUM_IMPL_LOOP_207(M, C, D, __VA_ARGS__)

#define WISE_ENUM_IMPL_LOOP_209(M, C, D, x, ...) M(C, x) D()                            \
  WISE_ENUM_IMPL_LOOP_208(M, C, D, __VA_ARGS__)

#define WISE_ENUM_IMPL_LOOP_210(M, C, D, x, ...) M(C, x) D()                            \
  WISE_ENUM_IMPL_LOOP_209(M, C, D, __VA_ARGS__)

#define WISE_ENUM_IMPL_LOOP_211(M, C, D, x, ...) M(C, x) D()                            \
  WISE_ENUM_IMPL_LOOP_210(M, C, D, __VA_ARGS__)

#define WISE_ENUM_IMPL_LOOP_212(M, C, D, x, ...) M(C, x) D()                            \
  WISE_ENUM_IMPL_LOOP_211(M, C, D, __VA_ARGS__)

#define WISE_ENUM_IMPL_LOOP_213(M, C, D, x, ...) M(C, x) D()                            \
  WISE_ENUM_IMPL_LOOP_212(M, C, D, __VA_ARGS__)

#define WISE_ENUM_IMPL_LOOP_214(M, C, D, x, ...) M(C, x) D()                            \
  WISE_ENUM_IMPL_LOOP_213(M, C, D, __VA_ARGS__)

#define WISE_ENUM_IMPL_LOOP_215(M, C, D, x, ...) M(C, x) D()                            \
  WISE_ENUM_IMPL_LOOP_214(M, C, D, __VA_ARGS__)

#define WISE_ENUM_IMPL_LOOP_216(M, C, D, x, ...) M(C, x) D()                            \
  WISE_ENUM_IMPL_LOOP_215(M, C, D, __VA_ARGS__)

#define WISE_ENUM_IMPL_LOOP_217(M, C, D, x, ...) M(C, x) D()                            \
  WISE_ENUM_IMPL_LOOP_216(M, C, D, __VA_ARGS__)

#define WISE_ENUM_IMPL_LOOP_218(M, C, D, x, ...) M(C, x) D()                            \
  WISE_ENUM_IMPL_LOOP_217(M, C, D, __VA_ARGS__)

#define WISE_ENUM_IMPL_LOOP_219(M, C, D, x, ...) M(C, x) D()                            \
  WISE_ENUM_IMPL_LOOP_218(M, C, D, __VA_ARGS__)

#define WISE_ENUM_IMPL_LOOP_220(M, C, D, x, ...) M(C, x) D()                            \
  WISE_ENUM_IMPL_LOOP_219(M, C, D, __VA_ARGS__)

#define WISE_ENUM_IMPL_LOOP_221(M, C, D, x, ...) M(C, x) D()                            \
  WISE_ENUM_IMPL_LOOP_220(M, C, D, __VA_ARGS__)

#define WISE_ENUM_IMPL_LOOP_222(M, C, D, x, ...) M(C, x) D()                            \
  WISE_ENUM_IMPL_LOOP_221(M, C, D, __VA_ARGS__)

#define WISE_ENUM_IMPL_LOOP_223(M, C, D, x, ...) M(C, x) D()                            \
  WISE_ENUM_IMPL_LOOP_222(M, C, D, __VA_ARGS__)

#define WISE_ENUM_IMPL_LOOP_224(M, C, D, x, ...) M(C, x) D()                            \
  WISE_ENUM_IMPL_LOOP_223(M, C, D, __VA_ARGS__)

#define WISE_ENUM_IMPL_LOOP_225(M, C, D, x, ...) M(C, x) D()                            \
  WISE_ENUM_IMPL_LOOP_224(M, C, D, __VA_ARGS__)

#define WISE_ENUM_IMPL_LOOP_226(M, C, D, x, ...) M(C, x) D()                            \
  WISE_ENUM_IMPL_LOOP_225(M, C, D, __VA_ARGS__)

#define WISE_ENUM_IMPL_LOOP_227(M, C, D, x, ...) M(C, x) D()                            \
  WISE_ENUM_IMPL_LOOP_226(M, C, D, __VA_ARGS__)

#define WISE_ENUM_IMPL_LOOP_228(M, C, D, x, ...) M(C, x) D()                            \
  WISE_ENUM_IMPL_LOOP_227(M, C, D, __VA_ARGS__)

#define WISE_ENUM_IMPL_LOOP_229(M, C, D, x, ...) M(C, x) D()                            \
  WISE_ENUM_IMPL_LOOP_228(M, C, D, __VA_ARGS__)

#define WISE_ENUM_IMPL_LOOP_230(M, C, D, x, ...) M(C, x) D()                            \
  WISE_ENUM_IMPL_LOOP_229(M, C, D, __VA_ARGS__)

#define WISE_ENUM_IMPL_LOOP_231(M, C, D, x, ...) M(C, x) D()                            \
  WISE_ENUM_IMPL_LOOP_230(M, C, D, __VA_ARGS__)

#define WISE_ENUM_IMPL_LOOP_232(M, C, D, x, ...) M(C, x) D()                            \
  WISE_ENUM_IMPL_LOOP_231(M, C, D, __VA_ARGS__)

#define WISE_ENUM_IMPL_LOOP_233(M, C, D, x, ...) M(C, x) D()                            \
  WISE_ENUM_IMPL_LOOP_232(M, C, D, __VA_ARGS__)

#define WISE_ENUM_IMPL_LOOP_234(M, C, D, x, ...) M(C, x) D()                            \
  WISE_ENUM_IMPL_LOOP_233(M, C, D, __VA_ARGS__)

#define WISE_ENUM_IMPL_LOOP_235(M, C, D, x, ...) M(C, x) D()                            \
  WISE_ENUM_IMPL_LOOP_234(M, C, D, __VA_ARGS__)

#define WISE_ENUM_IMPL_LOOP_236(M, C, D, x, ...) M(C, x) D()                            \
  WISE_ENUM_IMPL_LOOP_235(M, C, D, __VA_ARGS__)

#define WISE_ENUM_IMPL_LOOP_237(M, C, D, x, ...) M(C, x) D()                            \
  WISE_ENUM_IMPL_LOOP_236(M, C, D, __VA_ARGS__)

#define WISE_ENUM_IMPL_LOOP_238(M, C, D, x, ...) M(C, x) D()                            \
  WISE_ENUM_IMPL_LOOP_237(M, C, D, __VA_ARGS__)

#define WISE_ENUM_IMPL_LOOP_239(M, C, D, x, ...) M(C, x) D()                            \
  WISE_ENUM_IMPL_LOOP_238(M, C, D, __VA_ARGS__)

#define WISE_ENUM_IMPL_LOOP_240(M, C, D, x, ...) M(C, x) D()                            \
  WISE_ENUM_IMPL_LOOP_239(M, C, D, __VA_ARGS__)

#define WISE_ENUM_IMPL_LOOP_241(M, C, D, x, ...) M(C, x) D()                            \
  WISE_ENUM_IMPL_LOOP_240(M, C, D, __VA_ARGS__)

#define WISE_ENUM_IMPL_LOOP_242(M, C, D, x, ...) M(C, x) D()                            \
  WISE_ENUM_IMPL_LOOP_241(M, C, D, __VA_ARGS__)

#define WISE_ENUM_IMPL_LOOP_243(M, C, D, x, ...) M(C, x) D()                            \
  WISE_ENUM_IMPL_LOOP_242(M, C, D, __VA_ARGS__)

#define WISE_ENUM_IMPL_LOOP_244(M, C, D, x, ...) M(C, x) D()                            \
  WISE_ENUM_IMPL_LOOP_243(M, C, D, __VA_ARGS__)

#define WISE_ENUM_IMPL_LOOP_245(M, C, D, x, ...) M(C, x) D()                            \
  WISE_ENUM_IMPL_LOOP_244(M, C, D, __VA_ARGS__)

#define WISE_ENUM_IMPL_LOOP_246(M, C, D, x, ...) M(C, x) D()                            \
  WISE_ENUM_IMPL_LOOP_245(M, C, D, __VA_ARGS__)

#define WISE_ENUM_IMPL_LOOP_247(M, C, D, x, ...) M(C, x) D()                            \
  WISE_ENUM_IMPL_LOOP_246(M, C, D, __VA_ARGS__)

#define WISE_ENUM_IMPL_LOOP_248(M, C, D, x, ...) M(C, x) D()                            \
  WISE_ENUM_IMPL_LOOP_247(M, C, D, __VA_ARGS__)

#define WISE_ENUM_IMPL_LOOP_249(M, C, D, x, ...) M(C, x) D()                            \
  WISE_ENUM_IMPL_LOOP_248(M, C, D, __VA_ARGS__)

#define WISE_ENUM_IMPL_LOOP_250(M, C, D, x, ...) M(C, x) D()                            \
  WISE_ENUM_IMPL_LOOP_249(M, C, D, __VA_ARGS__)

#define WISE_ENUM_IMPL_LOOP_251(M, C, D, x, ...) M(C, x) D()                            \
  WISE_ENUM_IMPL_LOOP_250(M, C, D, __VA_ARGS__)

#define WISE_ENUM_IMPL_LOOP_252(M, C, D, x, ...) M(C, x) D()                            \
  WISE_ENUM_IMPL_LOOP_251(M, C, D, __VA_ARGS__)

#define WISE_ENUM_IMPL_LOOP_253(M, C, D, x, ...) M(C, x) D()                            \
  WISE_ENUM_IMPL_LOOP_252(M, C, D, __VA_ARGS__)

#define WISE_ENUM_IMPL_LOOP_254(M, C, D, x, ...) M(C, x) D()                            \
  WISE_ENUM_IMPL_LOOP_253(M, C, D, __VA_ARGS__)

#define WISE_ENUM_IMPL_LOOP_255(M, C, D, x, ...) M(C, x) D()                            \
  WISE_ENUM_IMPL_LOOP_254(M, C, D, __VA_ARGS__)

#define WISE_ENUM_IMPL_LOOP_256(M, C, D, x, ...) M(C, x) D()                            \
  WISE_ENUM_IMPL_LOOP_255(M, C, D, __VA_ARGS__)


#include <algorithm>
#include <array>
#include <type_traits>
#include <utility>

/*
 Macro interface

 The first argument to any macro enum is either the name of the enum, or a
 parenthesized pair, (name, storage). The first form results in unfixed
 underlying type for the enum, while the second form defines it explicitly.
 After that, the macros take a variadic enumerator list. Each entry in the list
 can be either a legal identifier name, or it can be a parenthesized pair where
 the first entry is the identifier and the second entry is the initializer.

 Usage notes:
   - A semi-colon after the macro invocation is not necessary, and your
     compiler may warn for it
*/

// Declare an enum at namespace scope
#define WISE_ENUM(name, ...) WISE_ENUM_IMPL(enum, name, , __VA_ARGS__)

// Declare an enum class at namespace scope
#define WISE_ENUM_CLASS(name, ...)                                             \
  WISE_ENUM_IMPL(enum class, name, , __VA_ARGS__)

// Declare an enum at class scope
#define WISE_ENUM_MEMBER(name, ...)                                            \
  WISE_ENUM_IMPL(enum, name, friend, __VA_ARGS__)

// Declare an enum class at class scope
#define WISE_ENUM_CLASS_MEMBER(name, ...)                                      \
  WISE_ENUM_IMPL(enum class, name, friend, __VA_ARGS__)

/*
 Adapt an existing enum into the wise enum API. This macro must be used at
 global scope. The first argument must be the name of the enum (qualified),
 followed by all the enumerators of the enum.
*/
#define WISE_ENUM_ADAPT(name, ...) WISE_ENUM_IMPL_ADAPT(name, __VA_ARGS__)

namespace wise_enum {

// Returns the string representation of an enumerator
template <class T>
constexpr string_type to_string(T t) {
  return wise_enum_detail_to_string(t, detail::Tag<T>{});
}

// Enumerators trait class. Each value is also available as a template variable
// for C++14 and on
template <class T>
struct enumerators {
  // For a given wise enum type, this variable allows iteration over enumerators
  // and their string names in the declared order. Each iterated object is a
  // struct with members { T value; string_type name; }
  static constexpr decltype(wise_enum_detail_array(detail::Tag<T>{})) range =
      wise_enum_detail_array(detail::Tag<T>{});

  // This variable is equal to the number of enumerators for the wise enum type.
  static constexpr std::size_t size = range.size();
};

template <class T>
constexpr decltype(
    wise_enum_detail_array(detail::Tag<T>{})) enumerators<T>::range;

template <class T>
constexpr std::size_t enumerators<T>::size;

#if __cplusplus >= 201402
template <class T>
constexpr auto &range = enumerators<T>::range;

template <class T>
constexpr std::size_t size = enumerators<T>::size;
#endif

// A type trait; this allows checking if a type is a wise_enum in generic code
template <class T>
using is_wise_enum = detail::is_wise_enum<T>;

#if __cplusplus >= 201402
template <class T>
static constexpr bool is_wise_enum_v = is_wise_enum<T>::value;
#endif

// Converts a string literal into a wise enum. Returns an optional<T>; if no
// enumerator has name matching the string, the optional is returned empty.
template <class T>
WISE_ENUM_CONSTEXPR_14 optional_type<T> from_string(string_type s) {
  auto it =
      std::find_if(enumerators<T>::range.begin(), enumerators<T>::range.end(),
                   [=](const detail::value_and_name<T> &x) {
                     return ::wise_enum::detail::compare(x.name, s);
                   });
  if (it == enumerators<T>::range.end())
    return {};

  return it->value;
}
} // namespace wise_enum


#include <type_traits>

namespace wise_enum {

template <class T>
constexpr bool is_enum_value(std::underlying_type_t<T> enum_integral) {
    const auto& r = range<T>;
    // range-for loop is not constexpr in 14
    for (std::size_t i = 0; i != r.size(); ++i) {
    if (static_cast<std::underlying_type_t<T>>(r[i].value) == enum_integral) {
      return true;
    }
  }
  return false;
}

/*
  Implementation note: this doesn't exactly implement the subset of the optional
  interface correctly, in particular operator* and value return by value. This
  is slightly unfortunate but all the other possibilities involve arguably worse
  tradeoffs:
   - Having an integral member and returning enum references would be UB, so to
  avoid UB and return references we have to store an enum
   - At this point, a careful reading of the standard is needed to see whether
  it is legal to store non enumerator values in an enum. As it turns out,
  storing underlying_type::max in an enum is not in general well defined, enums
  of implicit storage can only legally store values in the "bitset range" of
  their enumerators. Enum classes and explicit storage enums don't have this
  issue, but detecting explicit storage-ness is not possible, so it would mean
  only supporting enum class to avoid this UB
   - Finally, we can actually avoid all UB by use memcpy and memcmp instead of
  simple assignment and comparison. However, these functions are not constexpr,
  so we would be forced to un-constexpr many functions.

   In conclusion, returning by value isn't ideal, but discrepancies between its
  behavior and normal optional will mostly be caught at compile time (e.g. *foo
  = MyEnum::BAR will not compile for compact optional). None of the
  discrepancies are critical to usage.
 */
template <class T, std::underlying_type_t<T> invalid =
                       std::numeric_limits<std::underlying_type_t<T>>::max()>
class compact_optional {

  using integral_type = std::underlying_type_t<T>;

  static_assert(is_wise_enum_v<T>,
                "wise enum compact_optional is only for wise enum types");
  static_assert(
      !is_enum_value<T>(invalid),
      "Error, can't use this invalid value as it is taken by an enumerator");

public:
  compact_optional() = default;

  compact_optional(T t) : m_storage(static_cast<integral_type>(t)) {}

  constexpr T operator*() const { return static_cast<T>(m_storage); }

  constexpr bool has_value() const noexcept { return m_storage != invalid; }
  constexpr explicit operator bool() const noexcept {
    return m_storage != invalid;
  }

  constexpr T value() const {
    if (m_storage != invalid)
      return static_cast<T>(m_storage);
    else
      throw bad_optional_access{};
  }

  template <class U>
  constexpr T value_or(U &&u) {
    if (has_value())
      return static_cast<T>(m_storage);
    else
      return std::forward<U>(u);
  }

  void reset() noexcept { m_storage = invalid; }

  compact_optional(const compact_optional &other) = default;
  compact_optional(compact_optional &&other) = default;
  compact_optional &operator=(const compact_optional &other) = default;
  compact_optional &operator=(compact_optional &&other) = default;

private:
  integral_type m_storage = invalid;
};

} // namespace wise_enum
