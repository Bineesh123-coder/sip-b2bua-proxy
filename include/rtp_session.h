#ifndef RTP_SESSION_H
#define RTP_SESSION_H

#include <string>
#include <atomic>

struct RTPSession {
    std::string caller_ip;
    int caller_port;

    std::string callee_ip;
    int callee_port;

    int server_port;
    int sockfd = -1;

     // In your session header
    uint32_t caller_ip_n; // Store in network byte order
    uint32_t callee_ip_n;

    std::atomic<bool> running{true};
};

#endif