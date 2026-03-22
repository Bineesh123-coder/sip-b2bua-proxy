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
#include "sip_parser.h"
#include <map>
#include "call_session.h"
#include "call_session_manager.h"

class SIPServer : public Thread{
   
    private:
    bool m_bStopped;
    bool m_bStarted;
    char m_logString[2048];
    Clogger* m_pDailyLog;
    std::string logMsg;
    
    UDPSocket* m_pUDPSocket;
    void run();
    void processSipMessage(const std::string& data, const std::string& addr_ip, uword port);
    void processInviteMessage(const SIPParser& parser,const std::string &data,
                                     const std::string &ip,
                                     uword port);
    void processAckMessage(const SIPParser& parser, const std::string &sipMsg,
                                 const std::string& addr_ip,
                                 uword port);
    void processByeMessage(const SIPParser& parser,
                                 const std::string &sipMsg,
                                 const std::string& addr_ip,
                                 uword port);
    //void processOptionsMessage(const std::string &data, const std::string &ip);
    std::string build100Trying(const SIPParser& parser);

    void debug_testing();

    public:
    SIPServer();
    virtual ~SIPServer();
    void DeleteMemory();
    int Start();
    int Stop();
    std::string m_sDataPath;
    int m_debugLevel;
    int m_log;

    CallSessionManager m_sessionManager;
    

};
#endif