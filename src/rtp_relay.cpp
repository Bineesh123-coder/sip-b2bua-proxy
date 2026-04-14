#include "rtp_relay.h"
#include <arpa/inet.h>
#include <unistd.h>
#include <cstring>
#include <iostream>
#include <thread>
#include "udp_socket.h"


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

// void rtpRelayWorker(RTPSession &session)
// {
//     try {
//         UDPSocket rtpSocket(session.server_port, false, "0.0.0.0");
//         session.sockfd = rtpSocket.getSocketFD();
//         std::cout << "[RTP] Relay active on port " << session.server_port << std::endl;

//         char buffer[1500];
//         while (session.running) {
//             udword src_ip_raw;
//             uword src_port;
//             int size = rtpSocket.receive((sbyte*)buffer, sizeof(buffer), src_ip_raw, src_port);

//             if (size < 0) break; 
//             if (size < 12) continue; // Ignore non-RTP/tiny packets

//             struct in_addr addr; addr.s_addr = src_ip_raw;
//             std::string src_ip = inet_ntoa(addr);

//             // --- DYNAMIC LEARNING LOGIC ---
            
//             // 1. Is this the Caller? (Matches Caller's SIP IP)
//             if (src_ip == session.caller_ip) {
//                 // If the port changed from the SDP, update it (Symmetric RTP)
//                 if (session.caller_port != src_port) {
//                     session.caller_port = src_port;
//                 }
//                 // Forward to Callee
//                 if (!session.callee_ip.empty() && session.callee_port != 0) {
//                     rtpSocket.send(buffer, size, inet_addr(session.callee_ip.c_str()), session.callee_port);
//                     // Minimal logging (Uncomment for debugging, but remove for performance later)
//                     // std::cout << "RTP: Caller -> Callee (" << size << " bytes)" << std::endl;
//                 }
//             }
//             // 2. Is this the Callee? (Matches Callee's SIP IP)
//             else if (src_ip == session.callee_ip) {
//                 if (session.callee_port != src_port) {
//                     session.callee_port = src_port;
//                 }
//                 // Forward to Caller
//                 rtpSocket.send(buffer, size, inet_addr(session.caller_ip.c_str()), session.caller_port);
//                 // std::cout << "RTP: Callee -> Caller (" << size << " bytes)" << std::endl;
//             }
//             else {
//                 std::cout << "[RTP WARN] Unknown source: " << src_ip << ":" << src_port << std::endl;
//             }
//         }
//         std::cout << "[RTP] Relay thread exiting..." << std::endl;
//     }
//     catch (const std::exception &e) {
//         std::cout << "RTP Error: " << e.what() << std::endl;
//     }
// }

// void rtpRelayWorker(RTPSession &session) {
//     try {
//         std::cout << "[DEBUG] Thread entered for port " << session.server_port << std::endl;
//         UDPSocket rtpSocket(session.server_port, false, "0.0.0.0");
        
//         session.sockfd = rtpSocket.getSocketFD();
//         std::cout << "[RTP] Relay active on port " << session.server_port << std::endl;
//         // ... rest of loop
//     } catch (const std::exception &e) {
//         std::cerr << "[FATAL] RTP Thread failed to start: " << e.what() << std::endl;
//     }
// }
// void rtpRelayWorker(std::shared_ptr<RTPSession> session) { 
//     try {
//         // Access members using -> instead of .
//         std::cout << "[DEBUG] Thread entered for port " << session->server_port << std::endl;
//         UDPSocket rtpSocket(session->server_port, false, "0.0.0.0");
        
//         session->sockfd = rtpSocket.getSocketFD();
//         // ...
//     } catch (const std::exception &e) {
//         // ...
//     }
// }
// void rtpRelayWorker(std::shared_ptr<RTPSession> session) {
//     try {
//         // Use -> to access members of the shared_ptr
//         UDPSocket rtpSocket(session->server_port, false, "0.0.0.0");
//         session->sockfd = rtpSocket.getSocketFD();
        
//         // Loop logic here...
//     } catch (...) { /* ... */ }
// }

void rtpRelayWorker(std::shared_ptr<RTPSession> session)
{
    try
    {
        std::cout << "[RTP] Thread started on port " << session->server_port << std::endl;

        UDPSocket rtpSocket(session->server_port, false, "0.0.0.0");

        session->sockfd = rtpSocket.getSocketFD();
        session->running = true;

        char buffer[1500];

        udword src_ip_raw;
        uword  src_port;

        

        // while (session->running) {
        //     int size = rtpSocket.receive((sbyte*)buffer, sizeof(buffer), src_ip_raw, src_port);
        //     if (size <= 0) continue;

        //     uint8_t version = (buffer[0] >> 6) & 0x03;
        //     if (version != 2) continue;

          

        //     // Use thread-safe conversion
        //     char ip_str[INET_ADDRSTRLEN];
        //     inet_ntop(AF_INET, &src_ip_raw, ip_str, INET_ADDRSTRLEN);
        //     std::string src_ip(ip_str);

        //       std::cout << "[RTP CHECK] src=" << src_ip << ":" << src_port
        //   << " caller=" << session->caller_ip << ":" << session->caller_port
        //   << " callee=" << session->callee_ip << ":" << session->callee_port
        //   << std::endl;

        // //     if (src_ip == session->caller_ip) {
        // //         // We got audio from Caller -> Send to Callee
        // //         // Update port to handle Symmetric RTP/NAT
        // //         session->caller_port = src_port; 
        // //         std::cout << "[RELAY] Forwarding to Callee " << session->callee_ip 
        // //   << ":" << session->callee_port << " (" << size << " bytes)" << std::endl;
                
        // //         if (session->callee_port > 0) {
        // //             rtpSocket.send(buffer, size, inet_addr(session->callee_ip.c_str()), session->callee_port);
        // //         }
        // //     } 
        // //     else if (src_ip == session->callee_ip) {
        // //         // We got audio from Callee -> Send to Caller
        // //         session->callee_port = src_port;

        // //         if (session->caller_port > 0) {
        // //             rtpSocket.send(buffer, size, inet_addr(session->caller_ip.c_str()), session->caller_port);
        // //              std::cout << "[RELAY] Forwarding to Caller " << session->caller_ip 
        // //   << ":" << session->caller_port << " (" << size << " bytes)" << std::endl;
                
        // //         }
        // //     }

        //     // Caller → Callee
        //     if (src_ip == session->caller_ip) {
        //         // Handle symmetric RTP
        //         if (session->caller_port != src_port)
        //             session->caller_port = src_port;

        //         std::cout << "[FLOW] Caller -> Callee\n";

        //         rtpSocket.send(buffer, size,
        //             inet_addr(session->callee_ip.c_str()),
        //             session->callee_port);
        //     }

        //     // Callee → Caller
        //     else if (src_ip == session->callee_ip) {
        //         if (session->callee_port != src_port)
        //             session->callee_port = src_port;

        //         std::cout << "[FLOW] Callee -> Caller\n";

        //         rtpSocket.send(buffer, size,
        //             inet_addr(session->caller_ip.c_str()),
        //             session->caller_port);
        //     }

        //     // Unknown
        //     else {
        //         std::cout << "[RTP ERROR] Unknown source "
        //                 << src_ip << ":" << src_port << std::endl;
        //     }
        // }

        while (session->running) {
            int size = rtpSocket.receive((sbyte*)buffer, sizeof(buffer), src_ip_raw, src_port);
            if (size <= 0) continue;

            // Check RTP Version (Header check)
            if (((buffer[0] >> 6) & 0x03) != 2) continue;

            // NUMERIC COMPARISON (Fast and reliable)
            if (src_ip_raw == session->caller_ip_n) {
                // Forward Caller -> Callee
                if (session->caller_port != src_port) session->caller_port = src_port;
                
                rtpSocket.send(buffer, size, session->callee_ip_n, session->callee_port);
                 std::cout << "[FLOW] Caller -> Callee" << std::endl; 
            }
            else if (src_ip_raw == session->callee_ip_n) {
                // Forward Callee -> Caller
                if (session->callee_port != src_port) session->callee_port = src_port;

                rtpSocket.send(buffer, size, session->caller_ip_n, session->caller_port);
                 std::cout << "[FLOW] Callee -> Caller" << std::endl;
            }
            else {
                // To debug this, print the RAW hex value of the IPs
                std::printf("[RTP ERROR] Unknown Source: %08X vs Expected: %08X\n", 
                            src_ip_raw, session->caller_ip_n);
            }
        }
        std::cout << "[RTP] Thread exiting..." << std::endl;


        
    }
    catch (const std::exception &e)
    {
        std::cerr << "[RTP ERROR] " << e.what() << std::endl;
    }
}

