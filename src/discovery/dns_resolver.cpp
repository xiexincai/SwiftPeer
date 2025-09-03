#include "p2p/discovery/dns_resolver.h"
#include <regex>
#include <thread>

namespace p2p_core {

DnsResolver::DnsResolver() = default;

void DnsResolver::set_custom_resolver(std::function<std::string(const std::string&)> resolver) {
    custom_resolver_ = std::move(resolver);
}

std::string DnsResolver::resolve(const std::string& hostname) {
    if (custom_resolver_) {
        return custom_resolver_(hostname);
    }
    return default_resolve(hostname);
}

std::future<std::string> DnsResolver::resolve_async(const std::string& hostname) {
    return std::async(std::launch::async, [this, hostname]() {
        return resolve(hostname);
    });
}

bool DnsResolver::is_valid_ip(const std::string& ip) {
    // Simple IPv4 validation
    std::regex ipv4_pattern(R"(^(\d{1,3})\.(\d{1,3})\.(\d{1,3})\.(\d{1,3})$)");
    std::smatch match;
    
    if (std::regex_match(ip, match, ipv4_pattern)) {
        for (size_t i = 1; i <= 4; ++i) {
            int octet = std::stoi(match[i]);
            if (octet < 0 || octet > 255) {
                return false;
            }
        }
        return true;
    }
    
    return false;
}

std::string DnsResolver::default_resolve(const std::string& hostname) {
    // Simplified implementation, actual project should use system DNS resolution
    // Here returns the hostname itself, assuming it's already an IP address
    if (is_valid_ip(hostname)) {
        return hostname;
    }
    
    // For non-IP addresses, return a default value
    // In actual implementation, should call system DNS API here
    return "127.0.0.1";
}

} // namespace p2p_core


