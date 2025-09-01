#include "p2p/transport/quic_transport.h"
#include "p2p/utils/logging.h"
#include "p2p/data/protocol.h"

#ifdef P2P_WITH_MSQUIC

#include <msquic.h>
#include <memory>
#include <mutex>
#include <thread>
#include <vector>
#include <unordered_set>

namespace p2p_core {

static const char* kAlpn = "p2p/1";

class MsQuicEnv {
public:
  MsQuicEnv() {
    if (MsQuicOpen(&api_) != QUIC_STATUS_SUCCESS) api_ = nullptr;
    if (!api_) return;
    QUIC_REGISTRATION_CONFIG regCfg{ "p2p_core", QUIC_EXECUTION_PROFILE_LOW_LATENCY };
    if (api_->RegistrationOpen(&regCfg, &registration_) != QUIC_STATUS_SUCCESS) registration_ = nullptr;
    QUIC_BUFFER alpn{ static_cast<uint32_t>(strlen(kAlpn)), (uint8_t*)kAlpn };
    QUIC_SETTINGS settings{}; settings.IsSet.IdleTimeoutMs = TRUE; settings.IdleTimeoutMs = 30000;
    if (api_->ConfigurationOpen(registration_, &alpn, 1, &settings, sizeof(settings), nullptr, &config_) != QUIC_STATUS_SUCCESS) config_ = nullptr;
    if (config_) api_->ConfigurationLoadCredential(config_, &cred_);
  }
  ~MsQuicEnv() {
    if (config_) api_->ConfigurationClose(config_);
    if (registration_) api_->RegistrationClose(registration_);
    if (api_) MsQuicClose(api_);
  }
  const QUIC_API_TABLE* api() const { return api_; }
  HQUIC registration() const { return registration_; }
  HQUIC config() const { return config_; }
private:
  const QUIC_API_TABLE* api_{nullptr};
  HQUIC registration_{nullptr};
  HQUIC config_{nullptr};
  QUIC_CREDENTIAL_CONFIG cred_{}; // default (insecure for demo)
};

class QuicConnection;

class QuicStream : public IConnection, public std::enable_shared_from_this<QuicStream> {
public:
  QuicStream(const MsQuicEnv* env, HQUIC connection, HQUIC stream, Endpoint remote,
             FrameHandler onFrame, ConnHandler onClose)
      : env_(env), connection_(connection), stream_(stream), remote_(std::move(remote)),
        onFrame_(std::move(onFrame)), onClose_(std::move(onClose)) {}
  ~QuicStream() override { close(); }

  bool send(const std::vector<Byte>& data) override {
    if (!stream_) return false;
    auto buf = new QUIC_BUFFER{ static_cast<uint32_t>(data.size()), new uint8_t[data.size()] };
    std::memcpy(buf->Buffer, data.data(), data.size());
    if (env_->api()->StreamSend(stream_, buf, 1, QUIC_SEND_FLAG_ALLOW_0_RTT, buf) != QUIC_STATUS_SUCCESS) {
      delete[] buf->Buffer; delete buf; return false;
    }
    return true;
  }
  void close() override {
    auto s = stream_; stream_ = nullptr;
    if (s) env_->api()->StreamShutdown(s, QUIC_STREAM_SHUTDOWN_FLAG_ABORT, 0);
  }
  Endpoint remote_endpoint() const override { return remote_; }

  void on_stream_event(QUIC_STREAM_EVENT* Event) {
    switch (Event->Type) {
      case QUIC_STREAM_EVENT_RECEIVE: {
        for (uint32_t i = 0; i < Event->RECEIVE.BufferCount; ++i) {
          auto b = Event->RECEIVE.Buffers[i];
          buffer_.insert(buffer_.end(), b.Buffer, b.Buffer + b.Length);
          Frame f;
          while (try_decode_frame(buffer_, f)) {
            onFrame_(shared_from_this(), f);
          }
        }
        break;
      }
      case QUIC_STREAM_EVENT_SEND_COMPLETE: {
        auto clientBuf = reinterpret_cast<QUIC_BUFFER*>(Event->SEND_COMPLETE.ClientContext);
        if (clientBuf) { delete[] clientBuf->Buffer; delete clientBuf; }
        break;
      }
      case QUIC_STREAM_EVENT_SHUTDOWN_COMPLETE: {
        onClose_(shared_from_this());
        break;
      }
      default: break;
    }
  }

private:
  const MsQuicEnv* env_;
  HQUIC connection_{nullptr};
  HQUIC stream_{nullptr};
  Endpoint remote_;
  FrameHandler onFrame_;
  ConnHandler onClose_;
  std::vector<Byte> buffer_;
};

_IRQL_requires_max_(DISPATCH_LEVEL)
_Function_class_(QUIC_STREAM_CALLBACK)
static QUIC_STATUS QUIC_API StreamCallback(_In_ HQUIC /*Stream*/, _In_opt_ void* Context, _Inout_ QUIC_STREAM_EVENT* Event) {
  auto s = reinterpret_cast<QuicStream*>(Context);
  if (s) s->on_stream_event(Event);
  return QUIC_STATUS_SUCCESS;
}

class MsQuicTransport : public ITcpTransport {
public:
  explicit MsQuicTransport(const MsQuicEnv* env) : env_(env) {}
  ~MsQuicTransport() override { stop(); }

  bool listen(std::uint16_t port) override {
    if (env_->api()->ListenerOpen(env_->registration(), ListenerCallback, this, &listener_) != QUIC_STATUS_SUCCESS) return false;
    QUIC_ADDR addr{}; QuicAddrSetFamily(&addr, QUIC_ADDRESS_FAMILY_INET);
    QuicAddrSetPort(&addr, port);
    if (env_->api()->ListenerStart(listener_, &alpn_, 1, &addr) != QUIC_STATUS_SUCCESS) return false;
    boundPort_ = port; // can't easily query
    return true;
  }
  std::uint16_t bound_port() const override { return boundPort_; }
  void set_handlers(ConnHandler on_accept, FrameHandler on_frame, ConnHandler on_close) override {
    onAccept_ = std::move(on_accept); onFrame_ = std::move(on_frame); onClose_ = std::move(on_close);
  }
  ConnectionPtr connect(const Endpoint& ep, int /*timeoutMs*/) override {
    HQUIC conn = nullptr;
    if (env_->api()->ConnectionOpen(env_->registration(), ConnectionCallback, this, &conn) != QUIC_STATUS_SUCCESS) return nullptr;
    if (env_->api()->ConnectionStart(conn, env_->config(), QUIC_ADDRESS_FAMILY_UNSPEC, ep.host.c_str(), ep.port) != QUIC_STATUS_SUCCESS) {
      env_->api()->ConnectionClose(conn); return nullptr;
    }
    // Wait is omitted; open stream immediately; MsQuic will queue
    HQUIC stream = nullptr;
    if (env_->api()->StreamOpen(conn, QUIC_STREAM_OPEN_FLAG_NONE, StreamCallback, nullptr, &stream) != QUIC_STATUS_SUCCESS) {
      env_->api()->ConnectionClose(conn); return nullptr;
    }
    auto qs = std::make_shared<QuicStream>(env_, conn, stream, ep, onFrame_, onClose_);
    env_->api()->SetCallbackHandler(stream, (void*)StreamCallback, qs.get());
    env_->api()->StreamStart(stream, QUIC_STREAM_START_FLAG_IMMEDIATE);
    if (onAccept_) onAccept_(qs);
    return qs;
  }
  void stop() override {
    if (listener_) { env_->api()->ListenerStop(listener_); env_->api()->ListenerClose(listener_); listener_ = nullptr; }
  }

private:
  static QUIC_STATUS QUIC_API ConnectionCallback(HQUIC /*Connection*/, void* /*Context*/, QUIC_CONNECTION_EVENT* /*Event*/) {
    return QUIC_STATUS_SUCCESS;
  }
  static QUIC_STATUS QUIC_API ListenerCallback(HQUIC /*Listener*/, void* Context, QUIC_LISTENER_EVENT* Event) {
    auto self = reinterpret_cast<MsQuicTransport*>(Context);
    if (!self) return QUIC_STATUS_SUCCESS;
    switch (Event->Type) {
      case QUIC_LISTENER_EVENT_NEW_CONNECTION: {
        auto api = self->env_->api();
        api->SetCallbackHandler(Event->NEW_CONNECTION.Connection, (void*)ConnectionCallback, self);
        api->ConnectionSetConfiguration(Event->NEW_CONNECTION.Connection, self->env_->config());
        // Open a default bidirectional stream on accept
        HQUIC stream = nullptr;
        api->StreamOpen(Event->NEW_CONNECTION.Connection, QUIC_STREAM_OPEN_FLAG_NONE, StreamCallback, nullptr, &stream);
        Endpoint ep{"0.0.0.0", 0};
        auto qs = std::make_shared<QuicStream>(self->env_, Event->NEW_CONNECTION.Connection, stream, ep, self->onFrame_, self->onClose_);
        api->SetCallbackHandler(stream, (void*)StreamCallback, qs.get());
        api->StreamStart(stream, QUIC_STREAM_START_FLAG_NONE);
        if (self->onAccept_) self->onAccept_(qs);
        break;
      }
      default: break;
    }
    return QUIC_STATUS_SUCCESS;
  }

  const MsQuicEnv* env_;
  HQUIC listener_{nullptr};
  std::uint16_t boundPort_{0};
  ConnHandler onAccept_;
  FrameHandler onFrame_;
  ConnHandler onClose_;
  QUIC_BUFFER alpn_{ static_cast<uint32_t>(strlen(kAlpn)), (uint8_t*)kAlpn };
};

std::unique_ptr<ITcpTransport> make_quic_transport(const std::string& /*alpn*/) {
  static MsQuicEnv env;
  if (!env.api()) return nullptr;
  return std::make_unique<MsQuicTransport>(&env);
}

} // namespace p2p_core

#endif // P2P_WITH_MSQUIC


