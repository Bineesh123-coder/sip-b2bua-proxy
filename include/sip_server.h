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
#include <sstream>
#include <random>
#include <iomanip>

struct UserLocation
{
    std::string ip;
    uword port;
    time_t expiry;
};



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
    std::unordered_map<std::string, UserLocation> m_registrationDB;

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
    
    void processRegisterMessage(const SIPParser& parser,
                                 const std::string& addr_ip,
                                 uword port);
    std::string build100Trying(const std::shared_ptr<CallSession>& session);
    std::string buildRingingMsg(const std::shared_ptr<CallSession>& session);
    std::string build200OkMsg(const std::shared_ptr<CallSession>& session);
    std::string buildAckToCallee(const SIPParser& parser,
                                        const std::shared_ptr<CallSession>& session);
    std::string buildBye(const std::shared_ptr<CallSession>& session,
                                bool toCallee);
    std::string build200OkForBye(const SIPParser& parser);

    void debug_testing();

    //void startRTPRelay(RTPSession& rtp);
    //void startRTPRelay(std::shared_ptr<RTPSession> rtp);
    void startRTPRelay(std::shared_ptr<RTPSession> session);
    void stopRTPRelay(std::shared_ptr<RTPSession> session);
   //void stopRTPRelay(RTPSession& rtp); 
    void cleanupRegistrations();

    std::string CreateNewInviteMsg(const SIPParser& parser,
                                           const std::shared_ptr<CallSession>& session);
    std::string generateCallID();
    std::string generateTag();
    std::string generateBranch();
    std::string removeAllTags(const std::string& header);
    void call_summary(const std::shared_ptr<CallSession>& session);

    void onCallStart(const std::shared_ptr<CallSession>& session);
    void onCallConnected(const std::shared_ptr<CallSession>& session);
    void onCallEnd(const std::shared_ptr<CallSession>& session,
                         int duration,
                         int totalPackets,
                         int loss,
                         double avgJitter);
    
    public:
    SIPServer();
    virtual ~SIPServer();
    void DeleteMemory();
    int Start();
    int Stop();
    
};
#endif