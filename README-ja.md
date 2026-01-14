# cpp-uuidv7: C++17 UUIDv7 ライブラリ

**cpp-uuidv7** は、RFC 9562 UUIDv7 識別子を生成・パースする C++17 ライブラリです。UUIDv7 はミリ秒精度のタイムスタンプを持つ時間ベース UUID で、ソート可能かつ一意なため、対応アプリケーションに適しています。シンプルでスレッドセーフなクロスプラットフォーム設計です。

## 特徴

* UUIDv7 の生成・パース
* スレッドセーフな `uuidv7_generator` による並行生成対応
* 同一ミリ秒内での単調増加 ID 生成
* クロスプラットフォーム CSPRNG 利用 (OpenSSL, Windows BCrypt, Unix `getrandom`, BSD/macOS `arc4random_buf`)
* 文字列・バイト配列への容易な変換

## 要件

* C++17 互換コンパイラ
* CMake (3.22 以上)
* (オプション) OpenSSL (OpenSSL を乱数ソースとする場合)

## ライブラリのビルド

CMake を使用してビルドします。

1.  リポジトリをクローン: `git clone <repository-url> && cd cpp-uuidv7`
2.  ビルドディレクトリ作成・移動: `mkdir build && cd build`
3.  CMake 設定: `cmake .. [オプション]`
    * `-DUUIDV7LIB_TEST=ON`: テストビルド (GoogleTest 自動取得)
    * `-DUUIDV7LIB_DOCS=ON`: ドキュメントビルド (Doxygen 要)
    * `-DUUIDV7LIB_FORCE_NATIVE=ON`: ネイティブ CSPRNG の使用を強制
4.  ビルド: `cmake --build .`
5.  (オプション) テスト実行: `ctest` (テストビルド時)
6.  (オプション) インストール: `cmake --install . --prefix <場所>` (未指定時はデフォルト)

## ライブラリの使用方法

### UUIDv7 の生成
```cpp
#include <uuidv7/uuidv7.hpp>
#include <string>

int main() {
    // UUIDv7 を生成
    uuidv7::uuidv7_generator generator;
    uuidv7::uuidv7 id1 = generator.generate();

    // 文字列に変換
    std::string id1_str = id1.to_string();

    // バイト表現に変換
    std::array<uint8_t, 16> id1_bytes = id1.get_bytes();

    return 0;
}
```

### UUIDv7 文字列のパース
```cpp
#include <uuidv7/uuidv7.hpp>
#include <iostream>
#include <string>
#include <stdexcept>

int main() {
    std::string uuid_str = "018f7a6a-a1f1-72de-8000-000000000001";

    try {
        uuidv7::uuidv7 parsed_id = uuidv7::uuidv7::parse(uuid_str);
    } catch (const std::invalid_argument& e) {
        std::cerr << "Paese Error: " << e.what() << std::endl;
    } catch (const std::runtime_error& e) {
        std::cerr << "Runtime Error: " << e.what() << std::endl;
    }

    return 0;
}
```

## ライセンス
MIT License に準拠します。