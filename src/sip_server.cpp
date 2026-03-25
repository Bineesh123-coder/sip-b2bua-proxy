#include "sip_server.h"


SIPServer::SIPServer()
{
    try{
        memset(m_logString, 0, 2048);
        std::string logMsg="";
        m_bStopped = true;
        m_bStarted = false;
        m_pUDPSocket = nullptr;
        //m_pUDPSocket = new UDPSocket(5060, false);
        m_callsessionManager = nullptr;
        m_envReader = new EnvReader(); // Create a new environment reader instance
        std::cout<<"start SIPServer()\n";
    }
    catch(const std::exception &e)
    {
        std::cout<<"ERROR:constuctor SIPServer()\n";
    }
}
SIPServer::~SIPServer()
{
    try{
        
        DeleteMemory();
    }
    catch(const std::exception &e)
    {
        std::cout<<"ERROR:distructor SIPServer()\n";
    }
}
void SIPServer::DeleteMemory()
{
    try{
        if(m_pUDPSocket)
        {
            delete m_pUDPSocket;
            m_pDailyLog->WriteLog(kGeneralError, "SIPServer::DeleteMemory(): Deleted m_pUDPSocket");
        }
        m_pUDPSocket = nullptr;


        if(m_callsessionManager)
        {
            delete m_callsessionManager;
             m_pDailyLog->WriteLog(kGeneralError, "SIPServer::DeleteMemory(): Deleted m_callsessionManager");
        }
        m_callsessionManager = nullptr;
       

        if (m_envReader)
        {
            delete m_envReader;

            m_pDailyLog->WriteLog(kGeneralError, "SIPServer::DeleteMemory(): Deleted m_envReader");
        }
        m_envReader = nullptr; // Prevents dangling pointer 

        m_pDailyLog->WriteLog(kGeneralError, "================================END================================");

        if(m_pDailyLog)
        {
            delete m_pDailyLog;
        }
        m_pDailyLog = nullptr;
    }
    catch(const std::exception &e)
    {
        std::cout<<"ERROR:DeleteMemmory()\n";
        m_pUDPSocket = nullptr;
        m_callsessionManager = nullptr;
        m_envReader = nullptr;
        m_pDailyLog = nullptr;
    }
}

void SIPServer::run()
{
    try
    {
        snprintf(m_logString, 500, "SIPServer::Thread::run()");
        m_pDailyLog->WriteLog(kDebug, m_logString);

        while (!m_bStopped)
        {
            m_pDailyLog->CreateLog(); // Create a new log entry for each iteration of the loop

            sbyte buffer[4096];
            udword ip;
            uword port;

            int bytes = m_pUDPSocket->receive(buffer, sizeof(buffer) - 1, ip, port);
                if (bytes < 0)
                {
                    perror("recvfrom failed");
                    continue;
                }
                if (bytes > 0)
                {
                    buffer[bytes] = '\0';

                    std::string sipMsg((char*)buffer, bytes);

                    struct in_addr addr;
                    addr.s_addr = ip;

                    std::string ipStr = inet_ntoa(addr);

                    processSipMessage(sipMsg, ipStr,port);
                }
            
           
            // Sleep for a short duration to prevent high CPU usage
            std::this_thread::sleep_for(std::chrono::milliseconds(100)); // Adjust sleep time as needed

        }
    }
    catch (...)
    {
        m_pDailyLog->WriteLog(kGeneralError, "CSynwayRecorder::run(): Exception RUN method");
    }
}

int  SIPServer::Start()
{
    try
    {   
        if(m_bStopped)
        {   
            // m_pDailyLog = new Clogger(m_sDataPath, m_debugLevel, m_log, "Sip_Server");
            // printf("CREATING LOG FILE\n");
            // m_pDailyLog->CreateLog();
            // Read environment settings for the IP recorder
            if (ReadSipServerSettingsENV() == kFailure)
            {
                m_pDailyLog->WriteLog(kGeneralError, "SIPServer::start():: ERROR::ReadSipServerSettingsENV Failed");
                return kFailure;
            }
            m_pUDPSocket = new UDPSocket(m_nsip_port, false);
            m_callsessionManager = new CallSessionManager(m_pDailyLog);
             //thread start
            if (!Thread::start())
            {
                m_pDailyLog->WriteLog(kGeneralError, "CSynwayRecorder::start()::ERROR start() FAILED");
                return kFailure;
            }
            m_bStarted = true;
            m_bStopped = false;

            //debug_testing();


                
        }
        
    }
    catch (...)
    {
        m_pDailyLog->WriteLog(kGeneralError, "CSIPServer::Start(): Exception Start method\n");
        return kFailure;
    }
    return kSuccess;
}

int SIPServer::Stop()
{
    try
    {
        m_pDailyLog->WriteLog(kGeneralError, "SIPServer::stop()::Begin()");
        if (m_bStarted)
        {
            m_bStarted = false;
            m_bStopped = true;

        
            std::this_thread::sleep_for(std::chrono::milliseconds(200));
           

        }
    }
    catch (...)
    {
        m_pDailyLog->WriteLog(kFatalError, "SIPServer::stop()::Exception!");
        return kFailure;
    }
    return kSuccess;
}

/* Connect to redis server */
int SIPServer::Connect_to_Redis()
{
    try
    {
        // Connect to Redis server
        m_pContext = redisConnect(m_sRedisHost.c_str(), m_nRedisPort);
        if (m_pContext == nullptr || m_pContext->err) {
            if (m_pContext) {
                std::cerr << "CSynwayIPRecorder::Connect_to_Redis():Connection error: " << m_pContext->errstr << std::endl;
                m_pDailyLog->WriteLog(kGeneralError, "CSynwayIPRecorder::Connect_to_Redis():Redis Connection error: " + std::string(m_pContext->errstr));
                redisFree(m_pContext);
            }
            else {
                std::cerr << "CSynwayIPRecorder::Connect_to_Redis():Connection error: can't allocate redis context" << std::endl;
                m_pDailyLog->WriteLog(kGeneralError, "CSynwayIPRecorder::Connect_to_Redis():Redis Connection error: can't allocate redis context");
            }
            return kFailure;
        }
        else
        {
            m_pDailyLog->WriteLog(kGeneralError, "CSynwayIPRecorder::Connect_to_Redis():Redis Connection SUCCESS");
        }

		// ================================================================
        //  🔐 Authenticate if password is set (supports username + password)
        // ================================================================
        if (!m_sRedisPassword.empty())
        {
            redisReply* authReply = nullptr;

            if (!m_sRedisUsername.empty())
                authReply = (redisReply*)redisCommand(m_pContext, "AUTH %s %s",
                    m_sRedisUsername.c_str(), m_sRedisPassword.c_str());
            else
                authReply = (redisReply*)redisCommand(m_pContext, "AUTH %s",
                    m_sRedisPassword.c_str());

            if (authReply == nullptr)
            {
                std::cerr << "CSynwayRecorder::Connect_to_Redis(): AUTH command failed" << std::endl;
                m_pDailyLog->WriteLog(kGeneralError, "CSynwayRecorder::Connect_to_Redis(): Redis AUTH command failed.");
                redisFree(m_pContext);
                return kFailure;
            }

            if (authReply->type == REDIS_REPLY_ERROR)
            {
                std::cerr << "CSynwayRecorder::Connect_to_Redis(): AUTH error: " << authReply->str << std::endl;
                m_pDailyLog->WriteLog(kGeneralError, "CSynwayRecorder::Connect_to_Redis(): Redis AUTH error: " + std::string(authReply->str));
                freeReplyObject(authReply);
                redisFree(m_pContext);
                return kFailure;
            }

            m_pDailyLog->WriteLog(kGeneralError, "CSynwayRecorder::Connect_to_Redis(): Redis AUTH SUCCESS");
            freeReplyObject(authReply);
        }

        //ExecuteRedisCommand("PING");
        // Send PING command
        redisReply* reply = (redisReply*)redisCommand(m_pContext, "PING");
        if (reply)
        {
            switch (reply->type)
            {
            case REDIS_REPLY_STRING:
                if (reply->str)
                {
                    std::cout << "CSynwayIPRecorder::PING: " << reply->str << std::endl;
                    m_pDailyLog->WriteLog(kGeneralError, "CSynwayIPRecorder::Redis PING: " + std::string(reply->str));
                }
                else
                {
                    std::cerr << "CSynwayIPRecorder::PING: Reply is a string but empty." << std::endl;
                    m_pDailyLog->WriteLog(kGeneralError, "CSynwayIPRecorder::Redis PING: Reply is a string but empty.");
                }
                break;

            case REDIS_REPLY_STATUS:
                std::cout << "CSynwayIPRecorder::PING Status: " << reply->str << std::endl;
                m_pDailyLog->WriteLog(kGeneralError, "CSynwayIPRecorder::Redis PING Status: " + std::string(reply->str ? reply->str : "null"));
                break;

            case REDIS_REPLY_INTEGER:
                std::cout << "CSynwayIPRecorder::PING Integer Reply: " << reply->integer << std::endl;
                m_pDailyLog->WriteLog(kGeneralError, "CSynwayIPRecorder::Redis PING Integer Reply: " + std::to_string(reply->integer));
                break;

            case REDIS_REPLY_NIL:
                std::cout << "CSynwayIPRecorder::PING Reply is NIL." << std::endl;
                m_pDailyLog->WriteLog(kGeneralError, "CSynwayIPRecorder::Redis PING Reply is NIL.");
                break;

            case REDIS_REPLY_ERROR:
                std::cerr << "CSynwayIPRecorder::PING Error: " << reply->str << std::endl;
                m_pDailyLog->WriteLog(kGeneralError, "CSynwayIPRecorder::Redis PING Error: " + std::string(reply->str ? reply->str : "null"));
                break;

            case REDIS_REPLY_ARRAY:
                std::cout << "CSynwayIPRecorder::PING Array Reply:" << std::endl;
                for (size_t i = 0; i < reply->elements; ++i)
                {
                    if (reply->element[i] && reply->element[i]->str)
                    {
                        std::cout << "CSynwayIPRecorder::  Element[" << i << "]: " << reply->element[i]->str << std::endl;
                        m_pDailyLog->WriteLog(kGeneralError, "CSynwayIPRecorder::PING Array Element[" + std::to_string(i) + "]: " + std::string(reply->element[i]->str));
                    }
                }
                break;

            default:
                std::cerr << "CSynwayIPRecorder::PING: Unknown reply type." << std::endl;
                m_pDailyLog->WriteLog(kGeneralError, "CSynwayIPRecorder::Redis PING: Unknown reply type.");
                break;
            }
            freeReplyObject(reply);
        }
        else
        {
            std::cerr << "CSynwayIPRecorder::PING: Command failed. Possible Redis context or network issue." << std::endl;
            if (m_pContext && m_pContext->err)
            {
                std::cerr << "CSynwayIPRecorder::Error: " << m_pContext->errstr << std::endl;
                m_pDailyLog->WriteLog(kGeneralError, "CSynwayIPRecorder::Redis PING Error: " + std::string(m_pContext->errstr));
            }
            return kFailure;
        }
    }
    catch (...)
    {
        m_pDailyLog->WriteLog(kGeneralError, "CSynwayIPRecorder::Connect_to_Redis()::Exception.");
        return kFailure;
    }
    return kSuccess;
}


/* Reads IP recorder settings from environment variables */
int SIPServer::ReadSipServerSettingsENV()
{
    try
    {
        m_envReader->load(); //Load env

        m_sRedisHost = m_envReader->getValue("REDIS_HOST");
        printf(("REDIS_HOST:" + m_sRedisHost + "\n").c_str());

        m_nRedisPort = atoi(m_envReader->getValue("REDIS_PORT").c_str());
        printf(("REDIS_PORT:" + std::to_string(m_nRedisPort) + "\n").c_str());

		m_sRedisUsername = m_envReader->getValue("REDIS_USERNAME");
        printf(("REDIS_USERNAME:" + m_sRedisUsername + "\n").c_str());

        m_sRedisPassword = m_envReader->getValue("REDIS_PASSWORD");
        //printf(("REDIS_PASSWORD:" + m_sRedisPassword + "\n").c_str());

        m_sDataPath = m_envReader->getValue("DATADRIVE");
        printf(("DATADRIVE:" + m_sDataPath + "\n").c_str());

        m_debugLevel = atoi(m_envReader->getValue("LOGGER_STATUS").c_str());
        printf(("LOGGER_STATUS:" + std::to_string(m_debugLevel) + "\n").c_str());

        m_log = atoi(m_envReader->getValue("LOG").c_str());
        printf(("LOG:" + std::to_string(m_log) + "\n").c_str());

        m_pDailyLog = new Clogger(m_sDataPath, m_debugLevel, m_log, "Sip_Server");
        printf("CREATING LOG FILE\n");
        m_pDailyLog->CreateLog();
        m_envReader->SetLog(m_pDailyLog);

        int nRet = Connect_to_Redis();// connect to redis server
        if (nRet == kFailure)
        {
            return kFailure;
        }

        int kRet = ReadSipServerSettingsRedis(); // Read Synway SIPServerSettings from Redis

        if (kRet == kSuccess)
        {
            m_envReader->clearenvmap(); // Clear env map
            m_envReader->load();
            m_sDataPath = m_envReader->getValue("DATADRIVE");
            printf(("DATADRIVE:" + m_sDataPath + "\n").c_str());
            m_pDailyLog->SetDataPath(m_sDataPath);

            m_debugLevel = atoi(m_envReader->getValue("LOGGER_STATUS").c_str());
            printf(("LOGGER_STATUS:" + std::to_string(m_debugLevel) + "\n").c_str());
            m_pDailyLog->SetDebugLevel(m_debugLevel);

            m_log = atoi(m_envReader->getValue("LOG").c_str());
            printf(("LOG:" + std::to_string(m_log) + "\n").c_str());
            m_pDailyLog->SetLoggingEnabled(m_log);
        }
        snprintf(m_logString, 500, "SIPServer():ReadSipServerSettingsENV:: DATADRIVE[%s]", m_sDataPath.c_str());
        m_pDailyLog->WriteLog(kGeneralError, m_logString);
        snprintf(m_logString, 500, "SIPServer():ReadSipServerSettingsENV:: LOGGER_STATUS[%d]", m_debugLevel);
        m_pDailyLog->WriteLog(kGeneralError, m_logString);
        snprintf(m_logString, 500, "SIPServer():ReadSipServerSettingsENV:: LOG[%d]", m_log);
        m_pDailyLog->WriteLog(kGeneralError, m_logString);
        snprintf(m_logString, 500, "SIPServer():ReadSipServerSettingsENV:: REDIS_HOST[%s]", m_sRedisHost.c_str());
        m_pDailyLog->WriteLog(kGeneralError, m_logString);
        snprintf(m_logString, 500, "SIPServer():ReadSipServerSettingsENV:: REDIS_PORT[%d]", m_nRedisPort);
        m_pDailyLog->WriteLog(kGeneralError, m_logString);
		snprintf(m_logString, 500, "SIPServer():ReadSipServerSettingsENV:: REDIS_USERNAME[%s]", m_sRedisUsername.c_str());
        m_pDailyLog->WriteLog(kGeneralError, m_logString);

        m_EnvMap = m_envReader->getAll();

        m_sServer_ip = m_EnvMap["SERVER_IP"];
        printf(("SERVER_IP:" + m_sServer_ip + "\n").c_str());
        snprintf(m_logString, 500, "SIPServer():ReadSipServerSettingsENV:: SERVER_IP[%s]", m_sServer_ip.c_str());
        m_pDailyLog->WriteLog(kGeneralError, m_logString);
        
        m_nsip_port = atoi(m_EnvMap[(std::string)"SIP_PORT"].c_str());
        printf(("SIP_PORT:" + std::to_string(m_nsip_port) + "\n").c_str());
        snprintf(m_logString, 500, "SIPServer():ReadSipServerSettingsENV:: SIP_PORT[%d]", m_nsip_port);
        m_pDailyLog->WriteLog(kGeneralError, m_logString);

        m_nStart_rtp = atoi(m_EnvMap[(std::string)"START_RTP"].c_str());
        printf(("START_RTP:" + std::to_string(m_nStart_rtp) + "\n").c_str());
        snprintf(m_logString, 500, "SIPServer():ReadSipServerSettingsENV:: START_RTP[%d]", m_nStart_rtp);
        m_pDailyLog->WriteLog(kGeneralError, m_logString);

        m_nEnd_rtp = atoi(m_EnvMap[(std::string)"END_RTP"].c_str());
        printf(("END_RTP:" + std::to_string(m_nEnd_rtp) + "\n").c_str());
        snprintf(m_logString, 500, "SIPServer():ReadSipServerSettingsENV:: END_RTP[%d]",m_nEnd_rtp);
        m_pDailyLog->WriteLog(kGeneralError, m_logString);


        m_envReader->clearenvmap(); // Clear envmap

        return kSuccess;
    }
    catch (const std::exception& e)
    {
        if (m_pDailyLog)
        {
            m_pDailyLog->WriteLog(kGeneralError, "SIPServer::ReadSipServerSettingsENV(): Exception");

        }
        printf(("SIPServer::ReadSipServerSettingsENV(): Exception" + std::string(e.what()) + "\n").c_str());
    }
    return kFailure;
}


/* Read SIPServerSettings from Redis */
int SIPServer::ReadSipServerSettingsRedis()
{
    try
    {
        std::string hash = "SIP_SERVER_SETTINGS";
        redisReply* reply = nullptr;

        reply = (redisReply*)redisCommand(m_pContext, "HGETALL %s", hash.c_str());
        // Save the settings in the environment
        EnvReader envreader(m_pDailyLog);
        std::map<std::string, std::string> envMap; // Map to store key-value pairs
        envreader.load();
        envMap = envreader.getAll();

        // Iterate through the map and save values back using setValue()
        for (const auto& pair : envMap)
        {
            const std::string& key = pair.first;  // Get the key
            const std::string& value = pair.second;  // Get the value
            envreader.setValue(key, value);  // Save the key-value pair
        }

        if (reply && reply->type == REDIS_REPLY_ARRAY) {
            for (size_t i = 0; i < reply->elements; i += 2) 
            {
                std::string key = reply->element[i]->str;
                std::string value = reply->element[i + 1]->str;
                std::cout << key << ": " << value << std::endl;
                m_pDailyLog->WriteLog(kGeneralError, "SIPServerSettings::processMessage - " + key + ": " + value);
                printf(("SIPServerSettings::processMessage - " + key + ": " + value + "\n").c_str());

                if (!value.empty())
                {
                    envreader.setValue(key, value);
                }
            }
            envreader.save();
            envreader.clearenvmap();
            freeReplyObject(reply);
            return kSuccess;
        }
        else
        {   freeReplyObject(reply);
            return kFailure;
        }
    }
    catch (const std::exception& e)
    {
        if (m_pDailyLog)
        {
            m_pDailyLog->WriteLog(kGeneralError, "SIPServer::ReadSipServerSettingsRedis(): Exception");

        }
        printf(("SIPServer::ReadSIPServerSettingsRedis(): Exception" + std::string(e.what()) + "\n").c_str());
    }
    return kFailure;
}

void SIPServer::processSipMessage(const std::string& sipMsg,
                                  const std::string& addr_ip,
                                  uword port)
{
    try
    {
        SIPParser parser;
        parser.parse(sipMsg);

        std::string firstLine = parser.getRequestLine();
        std::string method    = parser.getMethod();


        // ==============================
        // 🔵 HANDLE RESPONSE (SIP/2.0)
        // ==============================
        if (firstLine.find("SIP/2.0") == 0)
        {
            std::string callID = parser.getCallID();
            CallSession* session = m_callsessionManager->getSession(callID);

            if (!session)
            {
                logMsg = "[WARN] Session not found";
                std::cout << logMsg << std::endl;
                return;
            }

            std::string statusLine = firstLine;
            std::string cseqm = parser.getCSeqMethod();

            std::string finalMsg = sipMsg;

            //  HANDLE 200 OK (INVITE)
            if (statusLine.find("200") != std::string::npos &&
                cseqm.find("INVITE") != std::string::npos)
            {
                std::string sdp = SDPParser::extractSDP(sipMsg);
                std::string headers = SDPParser::extractHeaders(sipMsg);

                sdp = SDPParser::cleanSDP(sdp);

                SDPInfo callee = SDPParser::parse(sdp);

                // store callee RTP
                session->rtp.callee_ip = callee.ip;
                session->rtp.callee_port = callee.port;

                // allocate RTP port
                if (session->rtp.server_port == 0)
                {
                    session->rtp.server_port = allocateRTPPort();
                }

                // modify SDP
                std::string modifiedSDP =
                     SDPParser::modifyAudioPort(sdp, session->rtp.server_port);

                modifiedSDP =
                    SDPParser::modifyConnectionIP(modifiedSDP, m_sServer_ip);

                headers =
                    SDPParser::updateContentLength(headers, modifiedSDP.size());

                finalMsg = headers + modifiedSDP;


                logMsg = "[CALL " + callID + "] SDP modified (200 OK)";
                std::cout << logMsg << std::endl;

                //  START RTP
                startRTPRelay(session->rtp);
            }

            //  FORWARD RESPONSE
            if (session->isCaller(addr_ip, port))
            {
                m_pUDPSocket->send(finalMsg.c_str(), finalMsg.length(),
                    inet_addr(session->targetIP.c_str()),
                    session->targetPort);
            }
            else
            {
                m_pUDPSocket->send(finalMsg.c_str(), finalMsg.length(),
                    inet_addr(session->callerIP.c_str()),
                    session->callerPort);
            }

            // ==============================
            //  STATE MACHINE
            // ==============================
            if (statusLine.find("180") != std::string::npos)
            {
                session->state = "RINGING";
            }
            else if (statusLine.find("200") != std::string::npos &&
                     cseqm.find("BYE") != std::string::npos)
            {
                session->state = "TERMINATED";
                stopRTPRelay(session->rtp);   
                m_callsessionManager->removeSession(callID);
            }
            else if (statusLine.find("487") != std::string::npos)
            {
                session->state = "TERMINATED";
                stopRTPRelay(session->rtp);   
                m_callsessionManager->removeSession(callID);
            }
            else if (statusLine.find("200") != std::string::npos)
            {
                session->state = "CONNECTED";
            }

            return; 
        }

        // ==============================
        // 🟢 HANDLE REQUESTS
        // ==============================

        if (method == "INVITE")
        {
            processInviteMessage(parser, sipMsg, addr_ip, port);
        }
        else if (method == "ACK")
        {
            processAckMessage(parser, sipMsg, addr_ip, port);
        }
        else if (method == "BYE")
        {
            processByeMessage(parser, sipMsg, addr_ip, port);
        }
        else if (method == "CANCEL")
        {
            processCancelMessage(parser, sipMsg, addr_ip, port);
        }
    }
    catch (const std::exception& e)
    {
        std::cout << "ERROR: " << e.what() << std::endl;
    }
}

void SIPServer::processInviteMessage(const SIPParser& parser,
                                     const std::string &data,
                                     const std::string &ip,
                                     uword port)
{
    try
    {
        logMsg = "INVITE received from " + ip + ":" + std::to_string(port);
        std::cout << logMsg << std::endl;
        m_pDailyLog->WriteLog(kDebug, logMsg);

        // 100 Trying
        std::string trying = build100Trying(parser);
        m_pUDPSocket->send(trying.c_str(), trying.length(),
                           inet_addr(ip.c_str()), port);
        
        //  CREATE SESSION FIRST
        auto session = std::make_shared<CallSession>();

        session->callID = parser.getCallID();
        session->callerIP = ip;
        session->callerPort = port;
        session->targetIP = "127.0.0.1";
        session->targetPort = 5061;
        session->state = "INVITE_SENT";

        // SDP
        std::string sdp = SDPParser::extractSDP(data);
        std::string headers = SDPParser::extractHeaders(data);

        SDPInfo caller = SDPParser::parse(sdp);

        // store caller RTP
        session->rtp.caller_ip = caller.ip;
        session->rtp.caller_port = caller.port;

        // allocate RTP port
        if (session->rtp.server_port == 0)
        {
            session->rtp.server_port = allocateRTPPort();
        }

        // modify SDP
        std::string modifiedSDP =
            SDPParser::modifyAudioPort(sdp, session->rtp.server_port);

        modifiedSDP =
            SDPParser::modifyConnectionIP(modifiedSDP, m_sServer_ip);

        // update content-length
        headers = SDPParser::updateContentLength(headers, modifiedSDP.size());

        std::string finalMsg = headers + modifiedSDP;

        // store session
        m_callsessionManager->addSession(session);

        // forward INVITE
        m_pUDPSocket->send(finalMsg.c_str(), finalMsg.length(),
                           inet_addr(session->targetIP.c_str()),
                           session->targetPort);

        logMsg = "[CALL " + session->callID + "] INVITE forwarded to CALLEE";
        std::cout << logMsg << std::endl;
        m_pDailyLog->WriteLog(kDebug, logMsg);
    }
    catch (const std::exception& e)
    {
        m_pDailyLog->WriteLog(kGeneralError,
            "ERROR:processInviteMessage " + std::string(e.what()));
    }
}

void SIPServer::processAckMessage(const SIPParser& parser,
                                 const std::string &sipMsg,
                                 const std::string& addr_ip,
                                 uword port)
{
    try {
        std::string callID = parser.getCallID();
        
        CallSession* session = m_callsessionManager->getSession(callID);

        if (!session)
        {
            logMsg ="[WARN] ACK: Session not found\n";
            std::cout<<logMsg<<std::endl;
            m_pDailyLog->WriteLog(kDebug, logMsg);
            return;
        }


            logMsg ="[CALL " + callID +"] ACK received from CALLER "
                  + addr_ip + ":" + std::to_string(port);
            std::cout<<logMsg<<std::endl;
            m_pDailyLog->WriteLog(kDebug, logMsg);

        //  Direction check
        if (session->isCaller(addr_ip,port))
        {
            // Caller → send to callee
            m_pUDPSocket->send(sipMsg.c_str(), sipMsg.length(),
                inet_addr(session->targetIP.c_str()),
                session->targetPort);

            logMsg ="[CALL " + callID +"] ACK forwarded to CALLEE";
            std::cout<<logMsg<<std::endl;
            m_pDailyLog->WriteLog(kDebug, logMsg);
        }
        else
        {
            // Callee → send to caller
            m_pUDPSocket->send(sipMsg.c_str(), sipMsg.length(),
                inet_addr(session->callerIP.c_str()),
                session->callerPort);

            logMsg = "[CALL " + callID +"] ACK forwarded to CALLER";
            std::cout<<logMsg<<std::endl;
            m_pDailyLog->WriteLog(kDebug, logMsg);
        }

      
    }
    catch (const std::exception& e)
    {
        m_pDailyLog->WriteLog(kGeneralError,
            "ERROR:processAckMessage " + std::string(e.what()));
    }
}

void SIPServer::processByeMessage(const SIPParser& parser,
                                 const std::string &sipMsg,
                                 const std::string& addr_ip,
                                 uword port)
{
    try {
        std::string callID = parser.getCallID();

        CallSession* session = m_callsessionManager->getSession(callID);

        if (!session)
        {
            logMsg ="[WARN] BYE: Session not found\n";
            std::cout<<logMsg<<std::endl;
            m_pDailyLog->WriteLog(kDebug, logMsg);
            return;
        }

        logMsg ="[CALL " + callID + "] BYE received from CALLER "
                  + addr_ip +":" + std::to_string(port);
        std::cout<<logMsg<<std::endl;
        m_pDailyLog->WriteLog(kDebug, logMsg);

        //  Direction check
        if (session->callerIP == addr_ip && session->callerPort == port)
        {
            // Caller → send to callee
            m_pUDPSocket->send(sipMsg.c_str(), sipMsg.length(),
                inet_addr(session->targetIP.c_str()),
                session->targetPort);

            logMsg ="[CALL " + callID + "] BYE forwarded to CALLEE";
            std::cout<<logMsg<<std::endl;
            m_pDailyLog->WriteLog(kDebug, logMsg);
        }
        else
        {
            // Callee → send to caller
            m_pUDPSocket->send(sipMsg.c_str(), sipMsg.length(),
                inet_addr(session->callerIP.c_str()),
                session->callerPort);

            logMsg ="[CALL " + callID + "] BYE forwarded to CALLER";
            std::cout<<logMsg<<std::endl;
            m_pDailyLog->WriteLog(kDebug, logMsg);
        }

        //  DO NOT DELETE SESSION HERE
        session->state = "TERMINATING";


    }
    catch (const std::exception& e)
    {
        m_pDailyLog->WriteLog(kGeneralError,
            "ERROR:processByeMessage " + std::string(e.what()));
    }
}

void SIPServer::processCancelMessage(const SIPParser& parser,
                                 const std::string &sipMsg,
                                 const std::string& addr_ip,
                                 uword port)
{
    try {

        

        std::string callID = parser.getCallID();

        CallSession* session = m_callsessionManager->getSession(callID);

        if (!session)
        {
            logMsg ="[WARN] CANCEL: Session not found\n";
            std::cout<<logMsg<<std::endl;
            m_pDailyLog->WriteLog(kDebug, logMsg);
            return;
        }

        logMsg ="[CALL " + callID + "] CANCEL received from CALLER "
                  + addr_ip +":" + std::to_string(port);
        std::cout<<logMsg<<std::endl;
        m_pDailyLog->WriteLog(kDebug, logMsg);

        std::string OK_200_msg = build_200_OK(parser);

        m_pUDPSocket->send(OK_200_msg.c_str(), OK_200_msg.length(),
                inet_addr(session->callerIP.c_str()),
                session->callerPort);

        logMsg = "Sending 200 ok message to "+ session->callerIP+":"+ std::to_string(session->callerPort);
        std::cout<<logMsg<<std::endl;
        m_pDailyLog->WriteLog(kDebug, logMsg);

        //  Direction check
        if (session->callerIP == addr_ip && session->callerPort == port)
        {
            // Caller → send to callee
            m_pUDPSocket->send(sipMsg.c_str(), sipMsg.length(),
                inet_addr(session->targetIP.c_str()),
                session->targetPort);

            logMsg ="[CALL " + callID + "] CANCEL forwarded to CALLEE";
            std::cout<<logMsg<<std::endl;
            m_pDailyLog->WriteLog(kDebug, logMsg);
        }
        else
        {
            // Callee → send to caller
            m_pUDPSocket->send(sipMsg.c_str(), sipMsg.length(),
                inet_addr(session->callerIP.c_str()),
                session->callerPort);

            logMsg ="[CALL " + callID + "] CANCEL forwarded to CALLER";
            std::cout<<logMsg<<std::endl;
            m_pDailyLog->WriteLog(kDebug, logMsg);
        }


    }
    catch (const std::exception& e)
    {
        m_pDailyLog->WriteLog(kGeneralError,
            "ERROR:processByeMessage " + std::string(e.what()));
    }
}


std::string SIPServer::build100Trying(const SIPParser& parser)
{   
    try{

        std::string response =
        "SIP/2.0 100 Trying\r\n"
        "Via: " + parser.getVia() + "\r\n"
        "From: " + parser.getFrom() + "\r\n"
        "To: " + parser.getTo() + "\r\n"
        "Call-ID: " + parser.getCallID() + "\r\n"
        "CSeq: " + parser.getCSeq() + "\r\n"
        "Content-Length: 0\r\n\r\n";

        return response;
    }
    catch (const std::exception &e)
    {
        std::cout << "ERROR: SIPServer::build100Trying: " << e.what() << std::endl;
        return "";
    }
    
}

std::string SIPServer::build_200_OK(const SIPParser& parser)
{
   
     try{

         std::string response =
        "SIP/2.0 200 OK\r\n"
        "Via: " + parser.getVia() + "\r\n"
        "From: " + parser.getFrom() + "\r\n"
        "To: " + parser.getTo() + "\r\n"
        "Call-ID: " + parser.getCallID() + "\r\n"
        "CSeq: " + parser.getCSeq() + "\r\n"
        "Content-Length: 0\r\n\r\n";

        return response;
    }
    catch (const std::exception &e)
    {
        std::cout << "ERROR: SIPServer:::build_200_OK: " << e.what() << std::endl;
        return "";
    }
    
}

void SIPServer::debug_testing()
{
    try
    {
        std::string sipMsg =
        "INVITE sip:7777@192.168.2.96:5060 SIP/2.0\r\n"
        "Via: SIP/2.0/UDP 192.168.2.28:5060;branch=z9hG4bK6a4ce70d46\r\n"
        "From: <sip:4002@192.168.2.28;x-nearend;x-refci=27757093;x-nearenddevice=SEP408D5CF67598;x-farendrefci=27757092;x-farenddevice=SEP34735AF87765;x-farendaddr=4006>;tag=98~209d1fb3\r\n"
        "To: <sip:7777@192.168.2.96>\r\n"
        "Call-ID: 5c8e6b80-9691eafc-25-1c02a8c0@192.168.2.28\r\n"
        "CSeq: 101 INVITE\r\n"
        "Content-Length: 0\r\n"
        "\r\n";

        // Simulated client IP & port
        std::string testIP = "192.168.1.10";
        uword testPort = 5060;

        processSipMessage(sipMsg, testIP, testPort);
    }
    catch (const std::exception &e)
    {
        std::cout << "ERROR: debug_testing(): " << e.what() << std::endl;
    }
}

//  FIX: use std::ref
void SIPServer::startRTPRelay(RTPSession &session) {
    try{

        std::thread(rtpRelayWorker, std::ref(session)).detach();
    }
    catch (const std::exception &e)
    {
        std::cout << "ERROR: SIPServer::startRTPRelay: " << e.what() << std::endl;
    }
    
}

void SIPServer::stopRTPRelay(RTPSession& rtp)
{
    
    try{

        rtp.running = false;

        if (rtp.sockfd > 0)
        {
            close(rtp.sockfd);   //  force unblock recvfrom
            std::cout << "[RTP] Stopped\n";
        }
    }
    catch (const std::exception &e)
    {
        std::cout << "ERROR: SIPServer::stopRTPRelay: " << e.what() << std::endl;
    }
}          
           