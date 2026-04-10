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
//     try{

//         UDPSocket rtpSocket(session.server_port, false);

//         session.sockfd = rtpSocket.getSocketFD();  // 🔥 ADD THIS

//         std::cout << "[RTP] Listening on port " << session.server_port << "\n";

//         char buffer[1500];

//         while (session.running)
//         {
//             udword ip;
//             uword port;

//             int size = rtpSocket.receive((sbyte*)buffer, sizeof(buffer), ip, port);

//             if (size <= 0) continue;

//             struct in_addr addr;
//             addr.s_addr = ip;
//             std::string src_ip = inet_ntoa(addr);
//             int src_port = port;

//             udword dest_ip;
//             uword dest_port;

//             if (src_port == session.caller_port)
//             {   
//                 session.caller_port = src_port;
//                 dest_ip = inet_addr(session.callee_ip.c_str());
//                 dest_port = session.callee_port;
//             }
//             else
//             {   
//                 session.callee_port = src_port;
//                 dest_ip = inet_addr(session.caller_ip.c_str());
//                 dest_port = session.caller_port;
//             }

//             rtpSocket.send(buffer, size, dest_ip, dest_port);
//         }

//         close(session.sockfd);  //  cleanup
//     }
//     catch (const std::exception &e)
//     {
//         std::cout << "ERROR: rtpRelayWorker: " << e.what() << std::endl;
//     }
    
// }

void rtpRelayWorker(RTPSession &session)
{
    try
    {
        UDPSocket rtpSocket(session.server_port, false,"0.0.0.0");
        

        std::string logMsg;
        session.sockfd = rtpSocket.getSocketFD();

        std::cout << "[RTP] Listening on port " << session.server_port << "\n";



        char buffer[1500];

        while (session.running)
        {
            udword ip;
            uword port;

            int size = rtpSocket.receive((sbyte*)buffer, sizeof(buffer), ip, port);

            if (size <= 0) continue;

            struct in_addr addr;
            addr.s_addr = ip;
            std::string src_ip = inet_ntoa(addr);
            uword src_port = port;

            udword dest_ip;
            uword dest_port;

            //  Identify by IP (NOT PORT)
            if (src_ip == session.caller_ip)
            {
                // Update port dynamically (Symmetric RTP)
                session.caller_port = src_port;

                dest_ip = inet_addr(session.callee_ip.c_str());
                dest_port = session.callee_port;

                std::cout << "[FLOW] Caller → Callee\n";
            }
            else
            {
                session.callee_port = src_port;

                dest_ip = inet_addr(session.caller_ip.c_str());
                dest_port = session.caller_port;

                std::cout << "[FLOW] Callee → Caller\n";
            }

            std::cout << "[RTP RECV] " << src_ip << ":" << src_port
                      << " size=" << size << std::endl;

            std::cout << "[RTP SEND] to "
                      << inet_ntoa(*(in_addr*)&dest_ip)
                      << ":" << dest_port << std::endl;

            rtpSocket.send(buffer, size, dest_ip, dest_port);
        }

        close(session.sockfd);
    }
    catch (const std::exception &e)
    {
        std::cout << "ERROR: rtpRelayWorker: " << e.what() << std::endl;
    }
}