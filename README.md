# maboroutu-appkit

[maboroutu](https://github.com/Eigen-0x41/maboroutu)（C++23/26 のモジュール式ユーティリティライブラリ）の利用側、つまり実行ファイルの入口（`main`）の周辺で使う補助部品を提供する。

## 位置づけ

- **役割:** プロセスの外側とのテキストによる接点の補助。現時点では、コマンドライン引数の振り分けと、診断出力の整形である。
- **含めないもの:** スレッド管理、アプリケーションの生存期間の管理、イベントループ。これらを持つ枠組み（フレームワーク）ではない。
- **依存:** 標準ライブラリのみ。`maboroutu`本体のモジュールには依存しない。
- **経緯:** `maboroutu` 0.0.3.8 までは、本体の`extension`ターゲットに含まれていた（`maboroutu.console`・`maboroutu.source_location_format`）。データ入出力を扱う本体と責務が異なるため、本リポジトリへ分離した。

## モジュール

モジュール名と名前空間（`maboroutu`）は、本体に含まれていた時点から変更していない。利用側は、依存先を`maboroutu::appkit`へ加えるだけでよい。

| モジュール | 内容 |
|---|---|
| `maboroutu.console` | コマンドライン引数のサブコマンドを、名前と処理関数の対応表で振り分ける`command_handler`と、`make_args_wrap` |
| `maboroutu.source_location_format` | `std::source_location`を`std::format`で整形する`std::formatter`の特殊化 |
| `maboroutu.result_format` | print debug 用。`std::expected`（`maboroutu::result`を含む）の値またはエラーの説明文字列、バイト列の16進表記を、`std::format`で整形する |

### `maboroutu.console`

```cpp
import maboroutu.console;

auto main(int argc, char *argv[]) -> int {
   auto argv_view = maboroutu::make_args_wrap(argc, argv);
   maboroutu::command_handler::args_type args(argv_view);

   maboroutu::command_handler::value_type commands;
   commands["hello"] = {
       .function = [](maboroutu::command_handler &,
                      maboroutu::command_handler::args_type args) {
          return static_cast<int>(args.size());
       },
       .description = "say hello",
   };

   maboroutu::command_handler handler(commands, args);
   return handler.run(args).value_or(1);
}
```

- `run(args)`は、`args[1]`以降の最初の「`-`で始まらない、空でない」引数をサブコマンド名として、対応する処理を実行し、その戻り値を返す。処理には`args.subspan(コマンド位置)`が渡される。見つからない、または未登録の場合は`std::nullopt`を返す。
- クラス名は`command_handler`である（`maboroutu` 0.0.3.8 までの`command_hander`は誤字であり、本リポジトリへの分離とは別に改名済み）。
- `maboroutu`本体の`command`（Undo/Redo 用のコマンド骨格。仕様のみで未実装）とは無関係である。

### `maboroutu.source_location_format`

`std::source_location`を`std::format`で整形するための包み型`maboroutu::formatted_location`と、その`std::formatter`の特殊化を提供する。書式は`ファイル名: 行, 列: 関数名`で、行と列は幅3で右寄せする（例: `main.cpp:  12,   5: int main()`）。

```cpp
import maboroutu.source_location_format;

auto text = std::format("{}", maboroutu::formatted_location{});   // 呼び出し位置
auto text2 = std::format("{}", maboroutu::formatted_location{loc}); // 既存の std::source_location
```

- 書式指定（`{:...}`）には対応しない。
- 標準ライブラリの型に対する`std::formatter`の特殊化を避けるため、包み型を用いる（`maboroutu` 0.0.3.8 までの`std::formatter<std::source_location>`は廃止した）。

## 既知の問題

- **`std::formatter`の`format`は、`format_context`を具象型で受けない。** Clang 18 + libc++ 18 では、`format(..., std::format_context &)`と書いた特殊化は`std::formattable`を満たさず、`std::format`が「the supplied type is not formattable」系のコンパイルエラーになる（モジュールの有無とは無関係）。本リポジトリでは`template <class FormatContext> auto format(..., FormatContext &) const`と書いている。

## 要件

- C++23 以上、CMake 3.28 以上、Ninja（C++ modules のため）。
- `maboroutu`本体と同じく、Clang 18 + libc++ を主な動作環境とする。GCC・MSVC での動作は、本リポジトリとしては未検証である。

## 使い方

```cmake
include(FetchContent)
FetchContent_Declare(maboroutu_appkit
   GIT_REPOSITORY https://github.com/Eigen-0x41/maboroutu-appkit.git
   GIT_TAG        <タグまたはコミット>)
FetchContent_MakeAvailable(maboroutu_appkit)

target_link_libraries(your_target PRIVATE maboroutu::appkit)
```

- 利用側も、`set(CMAKE_CXX_EXTENSIONS OFF)`を設定する。本リポジトリのモジュールは、コンパイラ拡張を無効にして構築されるため、利用側が有効のままだと、Clang では「module file ... cannot be loaded due to a configuration mismatch」のエラーになる（Clang 18 + libc++ で確認）。
- 利用側は、`set(CMAKE_CXX_SCAN_FOR_MODULES ON)`も設定する（`import`のみを含む翻訳単位を、モジュールの依存関係の走査の対象にするため。上記の確認はこの設定で行った）。

## 検証

テストは、本体と同じく、別リポジトリの [maboroutu-tests](https://github.com/Eigen-0x41/maboroutu-tests) に置く（`appkit/`ディレクトリ）。本リポジトリのCI（`.github/workflows/ci.yml`）は、`maboroutu-tests`の検証ワークフローを、本リポジトリのコミットを対象にして呼び出す。

## バージョン

`<メジャー>.<マイナー>.<リビジョン>.<ビルド>`の形式とする。現在は`0.0.0.1`である。CMake のバージョン互換性の判定は、暫定で`SameMinorVersion`としている（`maboroutu`本体の版管理の方針が決まった時点で見直す）。

## ライセンス

MIT（`LICENSE`を参照）。

### `maboroutu.result_format`

読み取った値やエラーを、print debug で出力するための整形である。標準ライブラリのみに依存し、`maboroutu` 本体の型は`import`しない（エラー型が`message()`を持つことのみを要求する）。

```cpp
import maboroutu.result_format;

auto value = maboroutu::read_value<maboroutu::endian::little, std::uint32_t>(src, 0);
std::println("{}", maboroutu::show(value));      // 値、または "error: <説明文字列>"
std::println("{:#x}", maboroutu::show(value));   // 書式指定は値の型へ渡される
std::println("{}", maboroutu::show(std::span<std::byte const>(bytes))); // "de ad be ef"
```

- `show()`は、元の値を参照で保持する。整形が終わるまで、元の値が有効であること。
- 本体の`core`は、標準出力・標準エラー出力へ書き込まない。出力は、利用側が行う。
