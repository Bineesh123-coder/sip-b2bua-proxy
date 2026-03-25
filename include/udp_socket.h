#ifndef UDPSocket_H
#define UDPSocket_H

#include <pthread.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <unistd.h>
#include <string>
#include <cstring>
#include <iostream>
#include <stdexcept>

#include "constants.h"
#include "types.h"

// typedef unsigned short uword;
// typedef unsigned long udword;
// typedef char sbyte;

class UDPSocket
{
public:
    // Constructors
    // Creates a UDP socket, binds it to the specified port, and sets it to non-blocking if specified.
    UDPSocket(uword portNo = 0, bool UnBlocking = false);
    UDPSocket(uword portNo, bool UnBlocking, std::string IPAddress);

    // Destructor
    // Closes the socket and destroys the mutex.
    virtual ~UDPSocket();

    // Opens a UDP socket and binds it to the specified port. Sets the socket to non-blocking if specified.
    int open(bool UnBlocking);
    int open(bool UnBlocking, std::string IPAddress);

    // Closes the UDP socket if it is open.
    void close();

    // Sends data to a specified IP address and port.
    int send(const char* buffer, udword bufSize, udword ip, uword port);

    // Receives data from the UDP socket.
    int receive(sbyte* buffer, uword bufSize, udword& ip, uword& port);

    // Checks if there is incoming data available.
    int isData();

    // Retrieves the local IP address.
    std::string getLocalIP();

    // Returns the source address of the last received packet.
    sockaddr_in GetSocketSourceAddress();

    // Returns the destination address of the last sent packet.
    sockaddr_in GetSocketDestinationAddress();
    int getSocketFD() ;

private:
    // Sets the socket to non-blocking mode.
    int unBlockSocket();

    // Member Variables
    uword portNo;
    int socket_fd;  // Socket descriptor for Linux (int)
    sockaddr_in destAddress;
    sockaddr_in sourceAddress;
    std::string localIP;
    pthread_mutex_t m_csCriticalSection;
};

#endif
