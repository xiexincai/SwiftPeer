#include "p2p/security/crypto.h"
#include <cstring>
#include <sstream>
#include <iomanip>
#ifdef P2P_WITH_OPENSSL
#include <openssl/evp.h>
#include <openssl/ec.h>
#include <openssl/pem.h>
#include <openssl/ecdsa.h>
#endif

namespace p2p_core {

// Minimal SHA-256 implementation (public domain style)
namespace {

struct Sha256Ctx {
  std::uint64_t total_len = 0;
  std::uint32_t state[8];
  std::uint8_t buffer[64];
  std::size_t buffer_len = 0;
};

static const std::uint32_t K[64] = {
  0x428a2f98,0x71374491,0xb5c0fbcf,0xe9b5dba5,0x3956c25b,0x59f111f1,0x923f82a4,0xab1c5ed5,
  0xd807aa98,0x12835b01,0x243185be,0x550c7dc3,0x72be5d74,0x80deb1fe,0x9bdc06a7,0xc19bf174,
  0xe49b69c1,0xefbe4786,0x0fc19dc6,0x240ca1cc,0x2de92c6f,0x4a7484aa,0x5cb0a9dc,0x76f988da,
  0x983e5152,0xa831c66d,0xb00327c8,0xbf597fc7,0xc6e00bf3,0xd5a79147,0x06ca6351,0x14292967,
  0x27b70a85,0x2e1b2138,0x4d2c6dfc,0x53380d13,0x650a7354,0x766a0abb,0x81c2c92e,0x92722c85,
  0xa2bfe8a1,0xa81a664b,0xc24b8b70,0xc76c51a3,0xd192e819,0xd6990624,0xf40e3585,0x106aa070,
  0x19a4c116,0x1e376c08,0x2748774c,0x34b0bcb5,0x391c0cb3,0x4ed8aa4a,0x5b9cca4f,0x682e6ff3,
  0x748f82ee,0x78a5636f,0x84c87814,0x8cc70208,0x90befffa,0xa4506ceb,0xbef9a3f7,0xc67178f2
};

inline std::uint32_t rotr(std::uint32_t x, std::uint32_t n){ return (x>>n) | (x<<(32-n)); }

void sha256_init(Sha256Ctx& ctx){
  ctx.total_len = 0; ctx.buffer_len = 0;
  ctx.state[0]=0x6a09e667; ctx.state[1]=0xbb67ae85; ctx.state[2]=0x3c6ef372; ctx.state[3]=0xa54ff53a;
  ctx.state[4]=0x510e527f; ctx.state[5]=0x9b05688c; ctx.state[6]=0x1f83d9ab; ctx.state[7]=0x5be0cd19;
}

void sha256_transform(Sha256Ctx& ctx, const std::uint8_t* data){
  std::uint32_t w[64];
  for(int i=0;i<16;++i){
    w[i] = (static_cast<std::uint32_t>(data[i*4])<<24) |
           (static_cast<std::uint32_t>(data[i*4+1])<<16) |
           (static_cast<std::uint32_t>(data[i*4+2])<<8) |
           (static_cast<std::uint32_t>(data[i*4+3]));
  }
  for(int i=16;i<64;++i){
    std::uint32_t s0 = rotr(w[i-15],7) ^ rotr(w[i-15],18) ^ (w[i-15]>>3);
    std::uint32_t s1 = rotr(w[i-2],17) ^ rotr(w[i-2],19) ^ (w[i-2]>>10);
    w[i] = w[i-16] + s0 + w[i-7] + s1;
  }
  std::uint32_t a=ctx.state[0], b=ctx.state[1], c=ctx.state[2], d=ctx.state[3];
  std::uint32_t e=ctx.state[4], f=ctx.state[5], g=ctx.state[6], h=ctx.state[7];
  for(int i=0;i<64;++i){
    std::uint32_t S1 = rotr(e,6)^rotr(e,11)^rotr(e,25);
    std::uint32_t ch = (e & f) ^ ((~e) & g);
    std::uint32_t temp1 = h + S1 + ch + K[i] + w[i];
    std::uint32_t S0 = rotr(a,2)^rotr(a,13)^rotr(a,22);
    std::uint32_t maj = (a & b) ^ (a & c) ^ (b & c);
    std::uint32_t temp2 = S0 + maj;
    h = g; g = f; f = e; e = d + temp1; d = c; c = b; b = a; a = temp1 + temp2;
  }
  ctx.state[0]+=a; ctx.state[1]+=b; ctx.state[2]+=c; ctx.state[3]+=d;
  ctx.state[4]+=e; ctx.state[5]+=f; ctx.state[6]+=g; ctx.state[7]+=h;
}

void sha256_update(Sha256Ctx& ctx, const std::uint8_t* data, std::size_t len){
  ctx.total_len += len;
  std::size_t i = 0;
  if (ctx.buffer_len){
    while (len && ctx.buffer_len < 64){ ctx.buffer[ctx.buffer_len++] = data[i++]; --len; }
    if (ctx.buffer_len == 64){ sha256_transform(ctx, ctx.buffer); ctx.buffer_len = 0; }
  }
  for (; len >= 64; len -= 64, i += 64) sha256_transform(ctx, data + i);
  while (len--) ctx.buffer[ctx.buffer_len++] = data[i++];
}

void sha256_final(Sha256Ctx& ctx, std::uint8_t out[32]){
  std::uint64_t bit_len = ctx.total_len * 8;
  // append 0x80 then pad zeros
  sha256_update(ctx, (const std::uint8_t*)"\x80", 1);
  std::uint8_t zero = 0x00;
  while (ctx.buffer_len != 56){ sha256_update(ctx, &zero, 1); }
  std::uint8_t lenbuf[8];
  for (int i = 0; i < 8; ++i) lenbuf[7 - i] = static_cast<std::uint8_t>((bit_len >> (8 * i)) & 0xFF);
  sha256_update(ctx, lenbuf, 8);
  for (int i = 0; i < 8; ++i){
    out[i*4]   = static_cast<std::uint8_t>((ctx.state[i] >> 24) & 0xFF);
    out[i*4+1] = static_cast<std::uint8_t>((ctx.state[i] >> 16) & 0xFF);
    out[i*4+2] = static_cast<std::uint8_t>((ctx.state[i] >> 8) & 0xFF);
    out[i*4+3] = static_cast<std::uint8_t>((ctx.state[i]) & 0xFF);
  }
}

std::string to_hex(const std::uint8_t* p, std::size_t n){
  std::ostringstream oss; oss<<std::hex<<std::setfill('0');
  for (std::size_t i=0;i<n;++i) oss<<std::setw(2)<<(int)p[i];
  return oss.str();
}

} // anon

std::vector<std::uint8_t> sha256_bytes(const std::uint8_t* data, std::size_t len){
  Sha256Ctx ctx; sha256_init(ctx); sha256_update(ctx, data, len);
  std::vector<std::uint8_t> out(32); sha256_final(ctx, out.data()); return out;
}

std::vector<std::uint8_t> sha256_bytes(const std::vector<std::uint8_t>& data){
  return sha256_bytes(data.data(), data.size());
}

std::string sha256_hex(const std::uint8_t* data, std::size_t len){
  auto d = sha256_bytes(data, len); return to_hex(d.data(), d.size());
}

std::string sha256_hex(const std::vector<std::uint8_t>& data){
  return sha256_hex(data.data(), data.size());
}

std::vector<std::uint8_t> hmac_sha256_bytes(const std::string& key, const std::uint8_t* data, std::size_t len){
  // RFC 2104 HMAC with SHA-256
  const std::size_t block = 64;
  std::vector<std::uint8_t> k(block, 0);
  if (key.size() > block) {
    auto kh = sha256_bytes(reinterpret_cast<const std::uint8_t*>(key.data()), key.size());
    std::memcpy(k.data(), kh.data(), kh.size());
  } else {
    std::memcpy(k.data(), key.data(), key.size());
  }
  std::vector<std::uint8_t> ipad(block, 0x36), opad(block, 0x5c);
  for (std::size_t i=0;i<block;++i) { ipad[i] ^= k[i]; opad[i] ^= k[i]; }

  // inner = SHA256(ipad || data)
  Sha256Ctx ictx; sha256_init(ictx);
  sha256_update(ictx, ipad.data(), ipad.size());
  sha256_update(ictx, data, len);
  std::uint8_t inner[32]; sha256_final(ictx, inner);

  // outer = SHA256(opad || inner)
  Sha256Ctx octx; sha256_init(octx);
  sha256_update(octx, opad.data(), opad.size());
  sha256_update(octx, inner, sizeof(inner));
  std::vector<std::uint8_t> out(32); sha256_final(octx, out.data());
  return out;
}

std::string hmac_sha256_hex(const std::string& key, const std::uint8_t* data, std::size_t len){
  auto d = hmac_sha256_bytes(key, data, len); return to_hex(d.data(), d.size());
}

std::string hmac_sha256_hex(const std::string& key, const std::string& data){
  return hmac_sha256_hex(key, reinterpret_cast<const std::uint8_t*>(data.data()), data.size());
}

#ifdef P2P_WITH_OPENSSL
static std::vector<unsigned char> hex_to_bytes(const std::string& hex){
  std::vector<unsigned char> out; out.reserve(hex.size()/2);
  for (size_t i=0;i+1<hex.size();i+=2){ unsigned int v; sscanf(hex.c_str()+i, "%02x", &v); out.push_back((unsigned char)v); }
  return out;
}
#endif

std::string ecdsa_p256_sign_der_hex(const std::string& privateKeyPemPath,
                                    const std::vector<std::uint8_t>& messageHash32){
#ifdef P2P_WITH_OPENSSL
  std::string outHex;
  FILE* f = fopen(privateKeyPemPath.c_str(), "rb"); if (!f) return outHex;
  EVP_PKEY* pkey = PEM_read_PrivateKey(f, nullptr, nullptr, nullptr); fclose(f);
  if (!pkey) return outHex;
  EVP_MD_CTX* ctx = EVP_MD_CTX_new();
  if (EVP_DigestSignInit(ctx, nullptr, EVP_sha256(), nullptr, pkey) != 1) { EVP_PKEY_free(pkey); EVP_MD_CTX_free(ctx); return outHex; }
  size_t siglen = 0;
  if (EVP_DigestSign(ctx, nullptr, &siglen, messageHash32.data(), messageHash32.size()) != 1) { EVP_PKEY_free(pkey); EVP_MD_CTX_free(ctx); return outHex; }
  std::vector<unsigned char> sig(siglen);
  if (EVP_DigestSign(ctx, sig.data(), &siglen, messageHash32.data(), messageHash32.size()) != 1) { EVP_PKEY_free(pkey); EVP_MD_CTX_free(ctx); return outHex; }
  sig.resize(siglen);
  outHex = to_hex(sig.data(), sig.size());
  EVP_PKEY_free(pkey); EVP_MD_CTX_free(ctx);
  return outHex;
#else
  (void)privateKeyPemPath; (void)messageHash32; return std::string();
#endif
}

bool ecdsa_p256_verify_der_hex(const std::string& publicKeyPem,
                               const std::vector<std::uint8_t>& messageHash32,
                               const std::string& derSigHex){
#ifdef P2P_WITH_OPENSSL
  BIO* bio = BIO_new_mem_buf(publicKeyPem.data(), (int)publicKeyPem.size());
  EVP_PKEY* pkey = PEM_read_bio_PUBKEY(bio, nullptr, nullptr, nullptr); BIO_free(bio);
  if (!pkey) return false;
  std::vector<unsigned char> sig = hex_to_bytes(derSigHex);
  EVP_MD_CTX* ctx = EVP_MD_CTX_new();
  if (EVP_DigestVerifyInit(ctx, nullptr, EVP_sha256(), nullptr, pkey) != 1) { EVP_PKEY_free(pkey); EVP_MD_CTX_free(ctx); return false; }
  int ok = EVP_DigestVerify(ctx, sig.data(), sig.size(), messageHash32.data(), messageHash32.size());
  EVP_PKEY_free(pkey); EVP_MD_CTX_free(ctx);
  return ok == 1;
#else
  (void)publicKeyPem; (void)messageHash32; (void)derSigHex; return false;
#endif
}

} // namespace p2p_core


