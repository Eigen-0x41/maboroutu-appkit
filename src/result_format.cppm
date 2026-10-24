module;
#include <concepts>
#include <cstddef>
#include <expected>
#include <format>
#include <span>
#include <string_view>
#include <type_traits>
export module maboroutu.result_format;

namespace maboroutu {

// print debug 用の包み型。`std::format`で整形する値を、参照で保持する。
// 整形が終わるまで、元の値が有効であること。
export template <class T> struct shown {
   T const &value;
};

export template <class T>
[[nodiscard]] constexpr auto show(T const &value) noexcept -> shown<T> {
   return {value};
}

} // namespace maboroutu

namespace maboroutu::result_format_detail {

// エラー型が、説明文字列を返す message() を持つこと（maboroutu::error 等）。
template <class E>
concept has_message = requires(E const &e) {
   { e.message() } -> std::convertible_to<std::string_view>;
};

template <class T> struct is_byte_span : std::false_type {};
template <std::size_t Extent>
struct is_byte_span<std::span<std::byte, Extent>> : std::true_type {};
template <std::size_t Extent>
struct is_byte_span<std::span<std::byte const, Extent>> : std::true_type {};

} // namespace maboroutu::result_format_detail

// std::expected<T, E>（maboroutu::result<T, Code> を含む）の整形。
// 値がある場合は、その値を T の書式で整形する（T が void の場合は "ok"）。
// エラーの場合は "error: <message()>" と整形する。
// 書式指定は、T の std::formatter へそのまま渡す。
template <class T, class E>
   requires maboroutu::result_format_detail::has_message<E>
struct std::formatter<maboroutu::shown<std::expected<T, E>>> {
   std::formatter<std::conditional_t<std::is_void_v<T>, int, T>> inner;

   constexpr auto parse(std::format_parse_context &ctx) {
      if constexpr (std::is_void_v<T>) {
         return ctx.begin();
      } else {
         return inner.parse(ctx);
      }
   }

   template <class FormatContext>
   auto format(maboroutu::shown<std::expected<T, E>> const &value,
               FormatContext &ctx) const {
      if (value.value.has_value()) {
         if constexpr (std::is_void_v<T>) {
            return std::format_to(ctx.out(), "ok");
         } else {
            return inner.format(*value.value, ctx);
         }
      }
      return std::format_to(
          ctx.out(), "error: {}",
          std::string_view(value.value.error().message()));
   }
};

// バイト列は、2桁の16進数を空白で区切って整形する（例: "de ad be ef"）。
template <class Span>
   requires maboroutu::result_format_detail::is_byte_span<Span>::value
struct std::formatter<maboroutu::shown<Span>> {
   constexpr auto parse(std::format_parse_context &ctx) { return ctx.begin(); }

   template <class FormatContext>
   auto format(maboroutu::shown<Span> const &value, FormatContext &ctx) const {
      auto out = ctx.out();
      bool first = true;
      for (auto const byte : value.value) {
         out = std::format_to(out, "{}{:02x}", first ? "" : " ",
                              static_cast<unsigned>(byte));
         first = false;
      }
      return out;
   }
};
