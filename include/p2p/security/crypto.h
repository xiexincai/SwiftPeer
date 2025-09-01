#pragma once

#include <string>
#include <vector>
#include <cstdint>

namespace p2p_core {

// SHA-256 digest utilities (no external deps)
std::vector<std::uint8_t> sha256_bytes(const std::uint8_t* data, std::size_t len);
std::vector<std::uint8_t> sha256_bytes(const std::vector<std::uint8_t>& data);
std::string sha256_hex(const std::uint8_t* data, std::size_t len);
std::string sha256_hex(const std::vector<std::uint8_t>& data);

// HMAC-SHA256 for tracker signing
std::vector<std::uint8_t> hmac_sha256_bytes(const std::string& key, const std::uint8_t* data, std::size_t len);
std::string hmac_sha256_hex(const std::string& key, const std::uint8_t* data, std::size_t len);
std::string hmac_sha256_hex(const std::string& key, const std::string& data);

// ECDSA P-256 signing/verification (OpenSSL required)
// Returns hex signature (DER encoded) or empty on failure
std::string ecdsa_p256_sign_der_hex(const std::string& privateKeyPemPath,
                                    const std::vector<std::uint8_t>& messageHash32);
bool ecdsa_p256_verify_der_hex(const std::string& publicKeyPem,
                               const std::vector<std::uint8_t>& messageHash32,
                               const std::string& derSigHex);

} // namespace p2p_core


