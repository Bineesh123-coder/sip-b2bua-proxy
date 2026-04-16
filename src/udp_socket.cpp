#include "udp_socket.h"
#include <netdb.h>
#include <fcntl.h>
#include <errno.h>

// Constructor to initialize and bind the socket
UDPSocket::UDPSocket(uword portNo, bool UnBlocking) : portNo(portNo), socket_fd(-1) {
    try {
        if (!open(UnBlocking)) {
            throw std::runtime_error("Socket open error");
        }
        destAddress.sin_family = AF_INET;
        sourceAddress.sin_family = AF_INET;
        pthread_mutex_init(&m_csCriticalSection, NULL);
    }
    catch (const std::exception& e) {
        std::cerr << "Exception in UDPSocket constructor: " << e.what() << std::endl;
    }
}

// Constructor with IP address
UDPSocket::UDPSocket(uword portNo, bool UnBlocking, std::string IPAddress) : portNo(portNo), socket_fd(-1) {
    try {
        if (!open(UnBlocking, IPAddress)) {
            throw std::runtime_error("Socket open error");
        }
        destAddress.sin_family = AF_INET;
        sourceAddress.sin_family = AF_INET;
        pthread_mutex_init(&m_csCriticalSection, NULL);
    }
    catch (const std::exception& e) {
        std::cerr << "Exception in UDPSocket constructor with IP: " << e.what() << std::endl;
    }
}

// Destructor to clean up resources
UDPSocket::~UDPSocket() {
    close();
    pthread_mutex_destroy(&m_csCriticalSection);
}

// Open a UDP socket and bind it to a port
int UDPSocket::open(bool UnBlocking) {
    try {
        socket_fd = ::socket(AF_INET, SOCK_DGRAM, 0);
        if (socket_fd == -1) {
            perror("socket");
            return 0;
        }

        sockaddr_in address = { };
        address.sin_addr.s_addr = htonl(INADDR_ANY);
        address.sin_family = AF_INET;
        address.sin_port = htons(portNo);

        if (bind(socket_fd, (struct sockaddr*)&address, sizeof(address)) == -1) {
            perror("bind");
            return 0;
        }

        if (UnBlocking) {
            unBlockSocket();
        }
    }
    catch (...) {
        std::cerr << "Error in open function" << std::endl;
        return 0;
    }
    return 1;
}

// Overloaded open method for binding to a specific IP address
int UDPSocket::open(bool UnBlocking, std::string IPAddress) {
    try {
        socket_fd = ::socket(AF_INET, SOCK_DGRAM, 0);
        if (socket_fd == -1) {
            perror("socket");
            return 0;
        }

        sockaddr_in address = { };
        address.sin_addr.s_addr = inet_addr(IPAddress.c_str());
        address.sin_family = AF_INET;
        address.sin_port = htons(portNo);

        if (bind(socket_fd, (struct sockaddr*)&address, sizeof(address)) == -1) {
            perror("bind");
            return 0;
        }

        if (UnBlocking) {
            unBlockSocket();
        }
    }
    catch (...) {
        std::cerr << "Error in open with IP function" << std::endl;
        return 0;
    }
    return 1;
}

// Closes the socket
void UDPSocket::close() {
    try {
        if (socket_fd != -1) {
            ::close(socket_fd);
            socket_fd = -1;
        }
    }
    catch (...) {
        std::cerr << "Error in close function" << std::endl;
    }
}

// Sends data to a specified IP and port
int UDPSocket::send(const char* buffer, udword bufSize, udword ip, uword port) {
    pthread_mutex_lock(&m_csCriticalSection);
    try {
        destAddress.sin_addr.s_addr = ip;
        destAddress.sin_port = htons(port);

        int ret = sendto(socket_fd, buffer, bufSize, 0, (struct sockaddr*)&destAddress, sizeof(destAddress));
        pthread_mutex_unlock(&m_csCriticalSection);
        return ret;
    }
    catch (...) {
        pthread_mutex_unlock(&m_csCriticalSection);
        std::cerr << "Error in send function" << std::endl;
        return -1;
    }
}

// Receives data from the socket
int UDPSocket::receive(sbyte* buffer, uword bufSize, udword& ip, uword& port) {
    try {
        socklen_t len = sizeof(sourceAddress);
        int ret = recvfrom(socket_fd, buffer, bufSize, 0, (struct sockaddr*)&sourceAddress, &len);
        ip = sourceAddress.sin_addr.s_addr;
        port = ntohs(sourceAddress.sin_port);
        return ret;
    }
    catch (...) {
        std::cerr << "Error in receive function" << std::endl;
        return -1;
    }
}

// Sets the socket to non-blocking mode
int UDPSocket::unBlockSocket() {
    try {
        int flags = fcntl(socket_fd, F_GETFL, 0);
        if (flags == -1) return -1;
        return fcntl(socket_fd, F_SETFL, flags | O_NONBLOCK);
    }
    catch (...) {
        std::cerr << "Error in unBlockSocket function" << std::endl;
        return -1;
    }
}

// Checks if there is data available to read
int UDPSocket::isData() {
    try {
        fd_set fds = { };
        struct timeval tv = { 0, 0 };
        FD_ZERO(&fds);
        FD_SET(socket_fd, &fds);
        return select(socket_fd + 1, &fds, NULL, NULL, &tv);
    }
    catch (...) {
        std::cerr << "Error in isData function" << std::endl;
        return -1;
    }
}

// Retrieves the local IP address
std::string UDPSocket::getLocalIP() {
    try {
        char host[256];
        gethostname(host, sizeof(host));

        struct hostent* localHost = gethostbyname(host);
        if (localHost) {
            localIP = inet_ntoa(*(struct in_addr*)localHost->h_addr_list[0]);
            return localIP;
        }
    }
    catch (...) {
        std::cerr << "Error in getLocalIP function" << std::endl;
    }
    return "";
}

sockaddr_in UDPSocket::GetSocketSourceAddress() {
    return sourceAddress;
}

sockaddr_in UDPSocket::GetSocketDestinationAddress() {
    return destAddress;
}

int UDPSocket::getSocketFD() { return this->socket_fd; }