#include "rtp_relay.h"
#include <arpa/inet.h>
#include <unistd.h>
#include <cstring>
#include <iostream>
#include <thread>
#include "udp_socket.h"

int allocateRTPPort() {
    static int port = 40000;
    port += 2;
    return port;
}

void rtpRelayWorker(RTPSession &session)
{
    UDPSocket rtpSocket(session.server_port, false);

    session.sockfd = rtpSocket.getSocketFD();  // 🔥 ADD THIS

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
        int src_port = port;

        udword dest_ip;
        uword dest_port;

        if (src_port == session.caller_port)
        {
            dest_ip = inet_addr(session.callee_ip.c_str());
            dest_port = session.callee_port;
        }
        else
        {
            dest_ip = inet_addr(session.caller_ip.c_str());
            dest_port = session.caller_port;
        }

        rtpSocket.send(buffer, size, dest_ip, dest_port);
    }

    close(session.sockfd);  //  cleanup
}
