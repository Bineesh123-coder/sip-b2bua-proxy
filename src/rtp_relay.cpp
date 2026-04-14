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
                // std::cout << "[FLOW] Caller -> Callee" << std::endl; 
            }
            else if (src_ip_raw == session->callee_ip_n) {
                // Forward Callee -> Caller
                if (session->callee_port != src_port) session->callee_port = src_port;

                rtpSocket.send(buffer, size, session->caller_ip_n, session->caller_port);
                // std::cout << "[FLOW] Callee -> Caller" << std::endl;
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

