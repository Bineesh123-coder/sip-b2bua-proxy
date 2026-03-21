#include <bits/stdc++.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <arpa/inet.h>

int main() {
    int sock = socket(AF_INET, SOCK_DGRAM, 0);
    sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_port = htons(5060);
    addr.sin_addr.s_addr = INADDR_ANY;
    bind(sock, (sockaddr*)&addr, sizeof(addr));

    char buffer[4096];
    std::cout << "SIP Proxy started on port 5060...\n";

    while(true) {
        sockaddr_in client;
        socklen_t len = sizeof(client);
        int bytes = recvfrom(sock, buffer, sizeof(buffer)-1, 0, (sockaddr*)&client, &len);
        buffer[bytes] = '\0';
        std::cout << "\n=== NEW SIP MESSAGE ===\n" << buffer << "\n";
    }
    close(sock);
    return 0;
}