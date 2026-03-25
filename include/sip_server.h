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

#include "rtp_session.h"
#include "sdp_parser.h"
#include "rtp_relay.h"
#include "env_reader.h"
#include <hiredis/hiredis.h>

class SIPServer : public Thread{
   
    private:
    bool m_bStopped;
    bool m_bStarted;
    char m_logString[2048];
    Clogger* m_pDailyLog;
    std::string logMsg;
    std::string m_serverIP;
    std::string m_sRedisHost;
    int  m_nRedisPort;
    std::string m_sRedisUsername;
    std::string m_sDataPath;
    int  m_debugLevel;
    int  m_log;
    int m_nsip_port;
    std::string m_sRedisPassword;
    std::string m_sServer_ip;
    int m_nStart_rtp;
    int m_nEnd_rtp;
    
    
    UDPSocket* m_pUDPSocket;
    CallSessionManager* m_callsessionManager;
    EnvReader* m_envReader;
    redisContext* m_pContext;
    std::map<std::string, std::string> m_EnvMap;

    void run();
    int ReadSipServerSettingsENV();
    int ReadSipServerSettingsRedis();
    int Connect_to_Redis();
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
    
    void processCancelMessage(const SIPParser& parser,
                                 const std::string &sipMsg,
                                 const std::string& addr_ip,
                                 uword port);
    std::string build100Trying(const SIPParser& parser);
    std::string build_200_OK(const SIPParser& parser);

    void debug_testing();

    void startRTPRelay(RTPSession& rtp);
    void stopRTPRelay(RTPSession& rtp); 

    

    public:
    SIPServer();
    virtual ~SIPServer();
    void DeleteMemory();
    int Start();
    int Stop();
    
};
#endif