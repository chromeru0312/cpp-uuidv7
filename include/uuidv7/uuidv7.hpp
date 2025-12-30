#ifndef UUIDV7_UUIDV7_HPP
#define UUIDV7_UUIDV7_HPP

#include <array>
#include <cassert>
#include <cstdint>
#include <mutex>
#include <sstream>
#include <string>

#ifdef UUIDV7LIB_SHARED
    #include "uuidv7lib_export.h"
#else
    #define UUIDV7_API
#endif

namespace uuidv7 {

class uuidv7_generator;

/// @brief UUIDv7 のシーケンスが同一ミリ秒内でオーバーフローした際に送出される例外
class UUIDV7_API sequence_overflow_error : public std::runtime_error {
public:
    sequence_overflow_error(const std::string& message) : std::runtime_error(message) {}
};

/// @brief UUIDv7 のパースエラーを表す例外クラス
class UUIDV7_API parse_error : public std::runtime_error {
public:
    parse_error(const std::string& message) : std::runtime_error(message) {}
};

/// @brief UUIDの値を表現する構造体
///
/// UUIDv7 はタイムスタンプベースであり、時間的順序性を持ちます。
/// 詳細は RFC 9562 Section 5.7 を参照してください。
///
/// @sa uuidv7_generator UUIDを生成するためのクラス
struct UUIDV7_API uuidv7 {
friend class uuidv7_generator;

public:
    /// @brief rand_a の最大値
    constexpr static std::uint16_t MAX_RAND_A = 0x0FFF;
    /// @brief rand_b の最大値
    constexpr static std::uint64_t MAX_RAND_B = 0x3FFFFFFFFFFFFFFF;

    /// @brief version (0b0111)
    constexpr static std::uint8_t VERSION = 7;
    /// @brief variant (0b10)
    constexpr static std::uint8_t VARIANT = 2;

    /// @brief UUIDv7を文字列からパースする
    /// @param str UUIDv7の文字列表現
    /// @return UUIDv7オブジェクト
    /// @throw std::invalid_argument 文字列がUUIDv7の形式に合致しない場合
    static uuidv7 parse(std::string const str);

    /// @brief UUIDv7を16バイトの配列から生成する
    /// @param bytes 16バイトの配列
    /// @return UUIDv7オブジェクト
    /// @throw std::invalid_argument バイト列がUUIDv7の形式に合致しない場合
    static uuidv7 from_bytes(const std::array<uint8_t, 16>& bytes);

    /// @brief UUIDv7を16バイトの配列から生成する
    /// @param bytes 16バイトの配列
    /// @return UUIDv7オブジェクト
    /// @throw std::invalid_argument バイト列がUUIDv7の形式に合致しない場合
    static uuidv7 from_bytes(const uint8_t* bytes);

    /// @brief UUIDv7を16バイトの配列として取得する
    /// @return std::array<uint8_t, 16> UUIDv7のバイト列表現
    std::array<uint8_t, 16> get_bytes() const noexcept;

    /// @brief UUIDv7を標準的な文字列表現に変換する
    /// @return UUIDv7の文字列表現
    std::string to_string() const;

private:
    /// @brief 48ビットのミリ秒単位Unixタイムスタンプ。
    std::uint64_t unix_ts_ms = 0;

    /// @brief 12ビットのランダムデータ (rand_a)。
    std::uint16_t rand_a = 0;

    /// @brief 62ビットのランダムデータ (rand_b)。
    std::uint64_t rand_b = 0;

    /// @brief 内部データからUUIDv7オブジェクトを構築するコンストラクタ
    /// @param ts_ms 48bit のミリ秒単位Unixタイムスタンプ
    /// @param r_a 12bit(`MAX_RAND_A` 以下) のランダムデータ
    /// @param r_b 62bit(`MAX_RAND_B` 以下) のランダムデータ
    /// @see MAX_RAND_A, MAX_RAND_B
    uuidv7(std::uint64_t ts_ms, std::uint16_t r_a, std::uint64_t r_b);

    friend bool operator==(const uuidv7& lhs, const uuidv7& rhs);
    friend bool operator<(const uuidv7& lhs, const uuidv7& rhs);

#ifndef NDEBUG
    /// @brief UUID文字列 (DEBUG only)
    mutable char uuid_string[37];
    /// @brief タイムスタンプの文字列表現 (DEBUG only)
    mutable char timestamp_string[25];

    /// @brief デバッグ用の変数を更新する (DEBUG only)
    void update_debug_variables() const noexcept;
#endif
};

/// @brief uuidv7 オブジェクトを生成するジェネレータクラス。
///
/// このクラスはスレッドセーフな UUIDv7 の生成機能を提供します。
///
/// 同じミリ秒内で複数のUUIDが生成された場合、ランダム部分のインクリメントにより、単調増加性を維持します。
///
/// @note このクラスは状態（最後に生成したUUID）を保持します。通常は一つのアプリケーションスレッドまたはプロセスで一つのインスタンスを共有します。
class UUIDV7_API uuidv7_generator {
public:
    /// @brief コンストラクタ
    uuidv7_generator() = default;

    /// @cond Doxygen_suppress
    // 状態のコピー防止のため、デフォルトコンストラクタと代入演算子を削除
    uuidv7_generator(const uuidv7_generator&) = delete;
    uuidv7_generator& operator=(const uuidv7_generator&) = delete;
    /// @endcond

    /// @brief デフォルトの uuidv7_generator インスタンスを取得する
    /// @return デフォルトの uuidv7_generator インスタンスへの参照
    static uuidv7_generator& default_instance() {
        static uuidv7_generator instance;
        return instance;
    }

    /// @brief デフォルトの uuidv7_generator インスタンスを使用して新しい uuidv7 オブジェクトを生成する
    ///
    /// 現在時刻に基づいてUUIDを生成します。
    /// 最後に生成したUUIDと同じミリ秒で生成された場合、ランダム部分をインクリメントします。
    ///
    /// この関数はスレッドセーフです。
    /// @return 生成された uuidv7 オブジェクト
    /// @throw std::system_error 乱数生成に失敗した (OS固有のCSPRNG呼び出しに失敗した) 場合
    /// @throw std::runtime_error 乱数生成に失敗した (OpenSSL等のCSPRNGライブラリ呼び出しに失敗した・エラーが取得できなかった) 場合
    /// @throw uuidv7::sequence_overflow_error 同一ミリ秒内で生成可能なUUIDの最大数を超えた場合
    static uuidv7 generate_default() { return default_instance().generate(); }

    /// @brief 新しい uuidv7 オブジェクトを生成する
    ///
    /// 現在時刻に基づいてUUIDを生成します。
    /// 最後に生成したUUIDと同じミリ秒で生成された場合、ランダム部分をインクリメントします。
    ///
    /// この関数はスレッドセーフです。
    /// @return 生成された uuidv7 オブジェクト
    /// @throw std::system_error 乱数生成に失敗した (OS固有のCSPRNG呼び出しに失敗した) 場合
    /// @throw std::runtime_error 乱数生成に失敗した (OpenSSL等のCSPRNGライブラリ呼び出しに失敗した・エラーが取得できなかった) 場合
    /// @throw uuidv7::sequence_overflow_error 同一ミリ秒内で生成可能なUUIDの最大数を超えた場合
    uuidv7 generate();

private:
    /// @brief generate() メソッドのスレッド安全性を確保するためのミューテックス
    std::mutex mutex_;

    /// @brief 最後に生成された uuidv7 オブジェクト
    uuidv7 last_generated_{0, 0, 0};

    /// @brief CSPRNGを使用してランダムな値を生成する
    /// @param a 12ビットのランダム値 (rand_a)
    /// @param b 62ビットのランダム値 (rand_b)
    void generate_random(std::uint16_t& a, std::uint64_t& b);
};

/// @cond Doxygen_suppress
// 等価・比較演算子
inline bool operator==(const uuidv7& lhs, const uuidv7& rhs) {
    return lhs.unix_ts_ms == rhs.unix_ts_ms && lhs.rand_a == rhs.rand_a && lhs.rand_b == rhs.rand_b;
}
inline bool operator!=(const uuidv7& lhs, const uuidv7& rhs) { return !(lhs == rhs); }

inline bool operator<(const uuidv7& lhs, const uuidv7& rhs) {
    return std::tie(lhs.unix_ts_ms, lhs.rand_a, lhs.rand_b) < std::tie(rhs.unix_ts_ms, rhs.rand_a, rhs.rand_b);
}
inline bool operator>(const uuidv7& lhs, const uuidv7& rhs) { return rhs < lhs; }
inline bool operator<=(const uuidv7& lhs, const uuidv7& rhs) { return !(rhs < lhs); }
inline bool operator>=(const uuidv7& lhs, const uuidv7& rhs) { return !(lhs < rhs); }
/// @endcond

} // namespace uuidv7

namespace std {
    template <>
    struct hash<uuidv7::uuidv7> {
        hash() = default;
        hash(const hash&) = default;

        size_t operator()(const uuidv7::uuidv7& u) const noexcept {
            return std::hash<std::string>()(u.to_string());
        }
    };
} // namespace std


#endif // UUIDV7_UUIDV7_HPP