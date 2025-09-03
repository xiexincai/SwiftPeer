#pragma once

#include <string>
#include <vector>
#include <cstdint>

namespace p2p_core {

// SHA-256摘要工具（无外部依赖）
std::vector<std::uint8_t> sha256_bytes(const std::uint8_t* data, std::size_t len);
std::vector<std::uint8_t> sha256_bytes(const std::vector<std::uint8_t>& data);
std::string sha256_hex(const std::uint8_t* data, std::size_t len);
std::string sha256_hex(const std::vector<std::uint8_t>& data);

// HMAC-SHA256用于跟踪器签名
std::vector<std::uint8_t> hmac_sha256_bytes(const std::string& key, const std::uint8_t* data, std::size_t len);
std::string hmac_sha256_hex(const std::string& key, const std::uint8_t* data, std::size_t len);
std::string hmac_sha256_hex(const std::string& key, const std::string& data);

// ECDSA P-256签名/验证（需要OpenSSL）
// 返回十六进制签名（DER编码）或失败时返回空字符串
std::string ecdsa_p256_sign_der_hex(const std::string& privateKeyPemPath,
                                    const std::vector<std::uint8_t>& messageHash32);
bool ecdsa_p256_verify_der_hex(const std::string& publicKeyPem,
                               const std::vector<std::uint8_t>& messageHash32,
                               const std::string& derSigHex);

} // namespace p2p_core


