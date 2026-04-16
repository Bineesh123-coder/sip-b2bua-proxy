#include "rtp_relay.h"
#include <arpa/inet.h>
#include <unistd.h>
#include <cstring>
#include <iostream>
#include <thread>
#include "udp_socket.h"

#include <arpa/inet.h>
#include <chrono>
#include <cmath>

int allocateRTPPort() {
    try{

        static int port = 40000;
        port += 2;
        return port;
    }
    catch (const std::exception &e)
    {
        std::cout << "ERROR: allocateRTPPort: " << e.what() << std::endl;
        return 0;

    }
    
}

void update_stats(StreamStats& stats, uint8_t* buffer) {
    stats.packet_count++;
    
    // Extract Sequence Number (Bytes 2-3) - Convert from Network Byte Order
    uint16_t seq = ntohs(*(uint16_t*)(buffer + 2));

    if (!stats.first_packet) {
        // Correct way to handle 16-bit sequence wrap-around
        uint16_t delta = seq - stats.last_seq;
        if (delta > 1 && delta < 3000) {
            stats.lost_packets += (delta - 1);
        }
    }
    
    // Simple Jitter (Time-based)
    auto now = std::chrono::steady_clock::now();
    int arrival = std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch()).count();
    uint32_t rtp_timestamp = ntohl(*(uint32_t*)(buffer + 4));
    
    // Note: To get accurate ms, you'd divide by clock rate (e.g., 8 for G.711)
    int transit = arrival - (rtp_timestamp / 8); 
    if (!stats.first_packet) {
        int d = std::abs(transit - stats.last_transit);
        stats.jitter += (d - stats.jitter) / 16.0;
    }
    
    stats.last_transit = transit;
    stats.last_seq = seq;
    stats.first_packet = false;
}

void rtpRelayWorker(std::shared_ptr<RTPSession> session)
{
    try {
        std::cout << "[RTP] Thread started on port " << session->server_port << std::endl;
        UDPSocket rtpSocket(session->server_port, false, "0.0.0.0");
        
        session->sockfd = rtpSocket.getSocketFD();
        session->running = true;
        session->start_time = std::chrono::steady_clock::now();

        uint8_t buffer[1500]; // Use uint8_t for easier byte manipulation
        udword src_ip_raw;
        uword  src_port;

        while (session->running) {
            int size = rtpSocket.receive((sbyte*)buffer, sizeof(buffer), src_ip_raw, src_port);
            
            if (size <= 0) continue;
            if (((buffer[0] >> 6) & 0x03) != 2) continue; // Version check

            // ROUTING & STATS LOGIC
            if (src_ip_raw == session->caller_ip_n) {
                // Direction: Caller -> Callee
                update_stats(session->caller_stats, buffer);
                
                if (session->caller_port != src_port) session->caller_port = src_port;
                
                if (session->callee_port > 0) {
                    rtpSocket.send(reinterpret_cast<const char*>(buffer), size, session->callee_ip_n, session->callee_port);
                }
            } 
            else if (src_ip_raw == session->callee_ip_n) {
                // Direction: Callee -> Caller
                update_stats(session->callee_stats, buffer);
                
                if (session->callee_port != src_port) session->callee_port = src_port;
                
                if (session->caller_port > 0) {
                    rtpSocket.send(reinterpret_cast<const char*>(buffer), size, session->caller_ip_n, session->caller_port);
                }
            } 
            else {
                // Unknown source - potential security risk or late packets
                // std::printf("[RTP BUG] Unknown source: %08X\n", src_ip_raw);
            }
        }
        
        session->end_time = std::chrono::steady_clock::now();
        std::cout << "[RTP] Thread exiting for port " << session->server_port << std::endl;
    }
    catch (const std::exception &e) {
        std::cerr << "[RTP CRASH] " << e.what() << std::endl;
    }
}

// void rtpRelayWorker(std::shared_ptr<RTPSession> session)
// {
//     try
//     {
//         std::cout << "[RTP] Thread started on port " << session->server_port << std::endl;

//         UDPSocket rtpSocket(session->server_port, false, "0.0.0.0");

//         session->sockfd = rtpSocket.getSocketFD();
//         session->running = true;

//         char buffer[1500];

//         udword src_ip_raw;
//         uword  src_port;


//         while (session->running) {
//             int size = rtpSocket.receive((sbyte*)buffer, sizeof(buffer), src_ip_raw, src_port);
//             if (size <= 0) continue;

//             // Check RTP Version (Header check)
//             if (((buffer[0] >> 6) & 0x03) != 2) continue;

//             // Inside the while (session->running) loop:

//             // 1. Increment packet count
//             session->packet_count++;

//             // 2. Extract Sequence Number (Bytes 2-3 of RTP Header)
//             uint16_t seq = (uint16_t)((buffer[2] << 8) | buffer[3]);

//             if (!session->first_packet) {
//                 // 3. Simple Packet Loss detection
//                 if (seq > (session->last_seq + 1)) {
//                     session->lost_packets += (seq - session->last_seq - 1);
//                 }
                
//                 // 4. Basic Jitter Calculation (Simplified)
//                 // In a real system, you use the RTP timestamp and arrival wall-clock time
//                 // J = J + (|D(i-1,i)| - J) / 16
//             }

//             session->last_seq = seq;
//             session->first_packet = false;

//             // NUMERIC COMPARISON (Fast and reliable)
//             if (src_ip_raw == session->caller_ip_n) {
//                 // Forward Caller -> Callee
//                 if (session->caller_port != src_port) session->caller_port = src_port;
                
//                 rtpSocket.send(buffer, size, session->callee_ip_n, session->callee_port);
//                 // std::cout << "[FLOW] Caller -> Callee" << std::endl; 
//             }
//             else if (src_ip_raw == session->callee_ip_n) {
//                 // Forward Callee -> Caller
//                 if (session->callee_port != src_port) session->callee_port = src_port;

//                 rtpSocket.send(buffer, size, session->caller_ip_n, session->caller_port);
//                 // std::cout << "[FLOW] Callee -> Caller" << std::endl;
//             }
//             else {
//                 // To debug this, print the RAW hex value of the IPs
//                 std::printf("[RTP ERROR] Unknown Source: %08X vs Expected: %08X\n", 
//                             src_ip_raw, session->caller_ip_n);
//             }
//         }
//         std::cout << "[RTP] Thread exiting..." << std::endl;


        
//     }
//     catch (const std::exception &e)
//     {
//         std::cerr << "[RTP ERROR] " << e.what() << std::endl;
//     }
// }

