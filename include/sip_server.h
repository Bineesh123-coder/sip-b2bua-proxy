#ifndef SIP_SERVER_H
#define SIP_SERVER_H

#include "constants.h"
#include <iostream>
#include "udp_socket.h"
#include <exception>
#include <string.h>
#include "thread.h"
#include "logger.h"
#include <thread>

class SIPServer : public Thread{
   
    private:
    bool m_bStopped;
    bool m_bStarted;
    char m_logString[2048];
    Clogger* m_pDailyLog;
    UDPSocket* m_pUDPSocket;
    void run();

    public:
    SIPServer();
    virtual ~SIPServer();
    void DeleteMemory();
    int Start();
    int Stop();
    std::string m_sDataPath;
    int m_debugLevel;
    int m_log;
    

};
#endif