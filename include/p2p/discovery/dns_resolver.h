#pragma once

#include <string>
#include <functional>
#include <future>

namespace p2p_core {

class DnsResolver {
public:
    DnsResolver();
    ~DnsResolver() = default;
    
    // 设置自定义解析回调
    void set_custom_resolver(std::function<std::string(const std::string&)> resolver);
    
    // 将主机名解析为IP地址
    std::string resolve(const std::string& hostname);
    
    // 异步解析主机名
    std::future<std::string> resolve_async(const std::string& hostname);
    
    // 检查是否为有效的IP地址
    static bool is_valid_ip(const std::string& ip);

private:
    std::function<std::string(const std::string&)> custom_resolver_;
    
    // 默认DNS解析实现
    std::string default_resolve(const std::string& hostname);
};

} // namespace p2p_core