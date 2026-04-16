#ifndef RTP_SESSION_H
#define RTP_SESSION_H

#include <string>
#include <atomic>
#include <chrono>

struct StreamStats {
    uint32_t packet_count = 0;
    uint16_t last_seq = 0;
    uint32_t lost_packets = 0;
    bool first_packet = true;
    double jitter = 0;
    int last_transit = 0;
};

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

    bool rtpStarted;

    // Inside your RTPSession class/struct
    StreamStats caller_stats;
    StreamStats callee_stats;

    std::chrono::steady_clock::time_point start_time;
    std::chrono::steady_clock::time_point end_time;

    std::atomic<bool> running{true};
};




#endif