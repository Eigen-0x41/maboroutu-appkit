module;
#include <format>
#include <source_location>
export module maboroutu.source_location_format;

namespace maboroutu {

// std::format で std::source_location を整形するための包み型。
// 生成時の既定引数により、生成位置の情報を保持する。
export struct formatted_location {
   std::source_location location;

   constexpr formatted_location(
       std::source_location loc = std::source_location::current()) noexcept
       : location(loc) {}
};

} // namespace maboroutu

// 書式: ファイル名: 行, 列: 関数名
template <> struct std::formatter<maboroutu::formatted_location> {
   constexpr auto parse(std::format_parse_context &ctx) { return ctx.begin(); }

   template <class FormatContext>
   auto format(maboroutu::formatted_location const &value,
               FormatContext &ctx) const {
      auto const &loc = value.location;
      return std::format_to(ctx.out(), "{}: {:>3}, {:>3}: {}", loc.file_name(),
                            loc.line(), loc.column(), loc.function_name());
   }
};
