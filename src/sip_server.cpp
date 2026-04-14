#include "sip_server.h"


SIPServer::SIPServer()
{
    try{
        memset(m_logString, 0, 2048);
        std::string logMsg="";
        m_bStopped = true;
        m_bStarted = false;
        m_pUDPSocket = nullptr;
        m_pUDPSocket = new UDPSocket(5060, false);
        m_callsessionManager = nullptr;
        m_envReader = new EnvReader(); // Create a new environment reader instance
        //m_serverIP ="192.168.2.225";
        m_sDataPath="/opt/app/DATA";
        m_log = 1;
        m_debugLevel= 40;
        //m_sServer_ip ="192.168.1.7";
        m_sServer_ip ="192.168.2.237";

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
        std::cout<<m_logString;
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

            cleanupRegistrations();
           
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
            m_pDailyLog = new Clogger(m_sDataPath, m_debugLevel, m_log, "Sip_Server");
            printf("CREATING LOG FILE\n");
            m_pDailyLog->CreateLog();
            // Read environment settings for the IP recorder
            // std::cout<<"hai"<<std::endl;
            // if (ReadSipServerSettingsENV() == kFailure)
            // {
            //     m_pDailyLog->WriteLog(kGeneralError, "SIPServer::start():: ERROR::ReadSipServerSettingsENV Failed");
            //     return kFailure;
            // }
            //m_pUDPSocket = new UDPSocket(m_nsip_port, false,"0.0.0.0");
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
                std::cerr << "SIPServer::Connect_to_Redis():Connection error: " << m_pContext->errstr << std::endl;
                m_pDailyLog->WriteLog(kGeneralError, "SIPServer::Connect_to_Redis():Redis Connection error: " + std::string(m_pContext->errstr));
                redisFree(m_pContext);
            }
            else {
                std::cerr << "SIPServer::Connect_to_Redis():Connection error: can't allocate redis context" << std::endl;
                m_pDailyLog->WriteLog(kGeneralError, "SIPServer::Connect_to_Redis():Redis Connection error: can't allocate redis context");
            }
            return kFailure;
        }
        else
        {
            m_pDailyLog->WriteLog(kGeneralError, "SIPServer::Connect_to_Redis():Redis Connection SUCCESS");
        }

		// ================================================================
        //   Authenticate if password is set (supports username + password)
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
                    std::cout << "SIPServer::PING: " << reply->str << std::endl;
                    m_pDailyLog->WriteLog(kGeneralError, "SIPServer::Redis PING: " + std::string(reply->str));
                }
                else
                {
                    std::cerr << "SIPServer::PING: Reply is a string but empty." << std::endl;
                    m_pDailyLog->WriteLog(kGeneralError, "SIPServer::Redis PING: Reply is a string but empty.");
                }
                break;

            case REDIS_REPLY_STATUS:
                std::cout << "SIPServer::PING Status: " << reply->str << std::endl;
                m_pDailyLog->WriteLog(kGeneralError, "SIPServer::Redis PING Status: " + std::string(reply->str ? reply->str : "null"));
                break;

            case REDIS_REPLY_INTEGER:
                std::cout << "SIPServer::PING Integer Reply: " << reply->integer << std::endl;
                m_pDailyLog->WriteLog(kGeneralError, "SIPServer::Redis PING Integer Reply: " + std::to_string(reply->integer));
                break;

            case REDIS_REPLY_NIL:
                std::cout << "SIPServer::PING Reply is NIL." << std::endl;
                m_pDailyLog->WriteLog(kGeneralError, "SIPServer::Redis PING Reply is NIL.");
                break;

            case REDIS_REPLY_ERROR:
                std::cerr << "SIPServer::PING Error: " << reply->str << std::endl;
                m_pDailyLog->WriteLog(kGeneralError, "SIPServer::Redis PING Error: " + std::string(reply->str ? reply->str : "null"));
                break;

            case REDIS_REPLY_ARRAY:
                std::cout << "SIPServer::PING Array Reply:" << std::endl;
                for (size_t i = 0; i < reply->elements; ++i)
                {
                    if (reply->element[i] && reply->element[i]->str)
                    {
                        std::cout << "SIPServer::  Element[" << i << "]: " << reply->element[i]->str << std::endl;
                        m_pDailyLog->WriteLog(kGeneralError, "SIPServer::PING Array Element[" + std::to_string(i) + "]: " + std::string(reply->element[i]->str));
                    }
                }
                break;

            default:
                std::cerr << "SIPServer::PING: Unknown reply type." << std::endl;
                m_pDailyLog->WriteLog(kGeneralError, "SIPServer::Redis PING: Unknown reply type.");
                break;
            }
            freeReplyObject(reply);
        }
        else
        {
            std::cerr << "SIPServer::PING: Command failed. Possible Redis context or network issue." << std::endl;
            if (m_pContext && m_pContext->err)
            {
                std::cerr << "SIPServer::Error: " << m_pContext->errstr << std::endl;
                m_pDailyLog->WriteLog(kGeneralError, "SIPServer::Redis PING Error: " + std::string(m_pContext->errstr));
            }
            return kFailure;
        }
    }
    catch (...)
    {
        m_pDailyLog->WriteLog(kGeneralError, "SIPServer::Connect_to_Redis()::Exception.");
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

/* Build ACK */
std::string SIPServer::buildAckToCallee(const SIPParser& parser,
                                        const std::shared_ptr<CallSession>& session)
{
    try{
        // Request line
        std::string requestLine =
            "ACK sip:" + parser.getToUser() + "@" +
            session->calleeIP + " SIP/2.0\r\n";

        // New Via (B2BUA creates its own)
        std::string via =
            "Via: SIP/2.0/UDP " + m_sServer_ip +
            ":5060;branch=" + session->calleeBranch + ";rport\r\n";

        // Build ACK
        std::string ack =
            requestLine +
            via +
            "Max-Forwards: 70\r\n"
            "From: " + session->calleeFrom + "\r\n" +   // caller side
            "To: " + session->calleeTo +";tag=" + session->calleetoTag+"\r\n"+
            "Call-ID: " + session->calleeCallID + "\r\n" + 
            "CSeq: 1 ACK\r\n"
            "Content-Length: 0\r\n\r\n";

        return ack;
    }
    catch (const std::exception &e)
    {
        m_pDailyLog->WriteLog(kGeneralError, "ERROR: SIPServer:: buildAckToCallee: " + std::string(e.what()));
        return "";
    }
}

/* Process sip message */
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
        //  HANDLE RESPONSES (SIP/2.0 ...)
        // ==============================
        if (firstLine.find("SIP/2.0") == 0)
        {   
            logMsg = "response sip msg\n"+sipMsg;
            std::cout << logMsg << std::endl;
            m_pDailyLog->WriteLog(kDebug, logMsg);
            
           // When a response comes in
            std::string callID = parser.getCallID();

            // You must check BOTH leg IDs because you don't know which leg 
            // the response is coming from until you find the session.
            auto session = m_callsessionManager->getSessionByCalleeCallID(callID);
            if (!session) {
                session = m_callsessionManager->getSessionByCallerCallID(callID);
            }

            if (!session)
            {
                logMsg = "[WARN] Session not found for response ";
                std::cout << logMsg << std::endl;
                m_pDailyLog->WriteLog(kDebug, logMsg);
                return;
            }

            std::string statusLine = firstLine;
            std::string cseqm      = parser.getCSeqMethod();

            // ==============================
            //  STATE MACHINE
            // ==============================
            if (statusLine.find("180") != std::string::npos)
            {
                
                std::string  ringing  = buildRingingMsg(session);
                m_pUDPSocket->send(ringing.c_str(), ringing.length(), inet_addr(session->callerIP.c_str()),session->callerPort);

                logMsg = "Sent ringing msg to caller  " + session->callerIP + ":" + std::to_string(session->callerPort)+"\n"+ringing;
                std::cout << logMsg << std::endl;
                m_pDailyLog->WriteLog(kDebug, logMsg);
                
                session->state = "RINGING";
                logMsg = "[STATE] RINGING";
            }
            else if (statusLine.find("200") != std::string::npos &&
                     cseqm.find("INVITE") != std::string::npos)
            {   
                session->calleetoTag = parser.getToTag();
                std::string  ok200msg = build200OkMsg(session);
                m_pUDPSocket->send(ok200msg.c_str(), ok200msg.length(), inet_addr(session->callerIP.c_str()), session->callerPort);

                logMsg = "Sent 200ok msg to caller  " + session->callerIP + ":" + std::to_string(session->callerPort)+"\n"+ok200msg;
                std::cout << logMsg << std::endl;
                m_pDailyLog->WriteLog(kDebug, logMsg);

                session->state = "CONNECTED";
                logMsg = "[STATE] CONNECTED";

                std::string calleeSdp = SDPParser::extractSDP(sipMsg);
                SDPInfo calleeInfo = SDPParser::parse(calleeSdp);


                // Store Callee's media destination
                session->rtp->callee_ip = session->calleeIP; 
                session->rtp->callee_port = calleeInfo.port;

                logMsg = "rtp callee ip " + session->rtp->callee_ip + " port " + std::to_string(session->rtp->callee_port)+"\n";
                std::cout << logMsg << std::endl;
                m_pDailyLog->WriteLog(kDebug, logMsg);

                session->rtp->caller_ip_n = inet_addr(session->rtp->caller_ip.c_str());
                session->rtp->callee_ip_n = inet_addr(session->rtp->callee_ip.c_str());

                session->rtp->running = true;
                logMsg = "[DEBUG] Calling startRTPRelay now...";
                //m_pDailyLog->WriteLog(kDebug, logMsg);
                startRTPRelay(session->rtp);

                 
            }
            else if (statusLine.find("200") != std::string::npos &&
                     cseqm.find("BYE") != std::string::npos)
            {
                session->state = "TERMINATED";
                //stopRTPRelay(session->rtp);   
                //m_callsessionManager->removeSession(callID);
                logMsg = "[STATE] TERMINATED (BYE)";

                session->byeConfirmedCount++;

                if (session->byeConfirmedCount == 2)
                {   
                    stopRTPRelay(session->rtp);
                    session->state = "TERMINATED";
                    logMsg = "[STATE] TERMINATED (BYE)";
                    m_callsessionManager->removeSession(callID);
                }
            }
            else if (statusLine.find("487") != std::string::npos)
            {
                session->state = "TERMINATED";
                m_callsessionManager->removeSession(callID);
                logMsg = "[STATE] TERMINATED (487)";
            }
            else if (statusLine.find("200") != std::string::npos)
            {
                // other 200 OK (ACK, etc.)
                session->state = "CONNECTED";
                logMsg = "[STATE] CONNECTED";
            }

            std::cout << logMsg << std::endl;
            m_pDailyLog->WriteLog(kDebug, logMsg);
            return; 
        }

        // ==============================
        //  HANDLE REQUESTS
        // ==============================
        if (method == "INVITE")
        {   
            logMsg = "INVITE msg : " + sipMsg;
            std::cout << logMsg << std::endl;
            m_pDailyLog->WriteLog(kDebug, logMsg);
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
        else if (method == "REGISTER")
        {
            processRegisterMessage(parser, addr_ip, port);
        }
    }
    catch (const std::exception& e)
    {
        m_pDailyLog->WriteLog(kGeneralError, "ERROR: SIPServer::processSipMessage: " + std::string(e.what()));
    }
}

// ===================================================================
// REGISTER 
// ===================================================================
void SIPServer::processRegisterMessage(const SIPParser& parser,
                                       const std::string& addr_ip,
                                       uword port)
{   
    try{

        std::string username = parser.getFromUser();
        std::string callID   = parser.getCallID();
        std::string via      = parser.getVia();
        std::string from     = parser.getFrom();
        std::string to       = parser.getTo();
        std::string cseq     = parser.getCSeq();

        int expires = 3600;
        std::string expHeader = parser.getHeader("Expires");
        if (!expHeader.empty())
            expires = std::stoi(expHeader);

        m_registrationDB[username] = { addr_ip, port, time(nullptr) + expires };

        logMsg = "[REGISTER] User: " + username + " IP: " + addr_ip + " Port: " + std::to_string(port);
        std::cout << logMsg << std::endl;
        m_pDailyLog->WriteLog(kDebug, logMsg);

        std::string response =
            "SIP/2.0 200 OK\r\n"
            "Via: " + via + "\r\n"
            "From: " + from + "\r\n"
            "To: " + to + ";tag=server123\r\n"
            "Call-ID: " + callID + "\r\n"
            "CSeq: " + cseq + "\r\n"
            "Contact: <sip:" + username + "@" + addr_ip + ":" + std::to_string(port) + ">\r\n"
            "Expires: 3600\r\n"
            "Content-Length: 0\r\n\r\n";

        m_pUDPSocket->send(response.c_str(), response.length(),
            inet_addr(addr_ip.c_str()), port);
    }
    catch (const std::exception& e)
    {
        m_pDailyLog->WriteLog(kGeneralError, "ERROR:SIPServer::processRegisterMessage: " + std::string(e.what()));
    }
    
}

// ===================================================================
// INVITE 
// ===================================================================
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

        std::string callID = parser.getCallID();
        if (m_callsessionManager->getSession(callID))
        {
            logMsg = "[INFO] Duplicate INVITE ignored";
            std::cout << logMsg << std::endl;
            return;
        }

        
        std::string callee = parser.getToUser();
        auto it = m_registrationDB.find(callee);
        if (it == m_registrationDB.end())
        {
            std::cout << "[ERROR] Callee not registered: " << callee << std::endl;
            std::string response = "SIP/2.0 404 Not Found\r\nContent-Length: 0\r\n\r\n";
            m_pUDPSocket->send(response.c_str(), response.length(), inet_addr(ip.c_str()), port);
            return;
        }
        std::string sdp     = SDPParser::extractSDP(data);
        // Create session
        auto session = std::make_shared<CallSession>();
        session->callerVia     = parser.getVia();
        session->callerFrom    = parser.getFrom();
        session->callerTo      = parser.getTo();
        session->callerCallID  = parser.getCallID();
        session->callerCSeq    = parser.getCSeq();
        session->callerIP      = ip;
        session->callerPort    = port;
        session->callersdp     = sdp;
        session->callerUser    = parser.getFromUser();
        session->callertoTag = generateTag();
        session->calleeCallID  = generateCallID();
        //session->calleeCallID = "18a4ecbad391a65277ca95695340b8c2@192.168.1.7";
        session->calleeIP      = it->second.ip;
        session->calleePort    = it->second.port;
        session->calleeUser    = parser.getToUser();
        // inside processInviteMessage...
        //session->callerFrom = parser.getFrom(); // e.g. <sip:1001@ip>;tag=abc
        //session->callerTo   = parser.getTo();   // e.g. <sip:1002@ip> (No tag yet)
        session->serverTagForCaller = generateTag(); // The tag the server uses toward the caller
        session->serverTagForCallee = generateTag(); // The tag the server uses toward the callee
        //session->calleetoTag = generateTag();
        session->state      = "INVITE_SENT";
        session->isTerminated = false;
        session->byeConfirmedCount = 0;
        m_callsessionManager->addSession(session);

        // 100 Trying
        std::string trying = build100Trying(session);
        m_pUDPSocket->send(trying.c_str(), trying.length(), inet_addr(ip.c_str()), port);

        logMsg = "Sent trying msg to caller  " + ip + ":" + std::to_string(port)+"\n"+trying;
        std::cout << logMsg << std::endl;
        m_pDailyLog->WriteLog(kDebug, logMsg);

        

        // SDP
        //std::string sdp     = SDPParser::extractSDP(data);
        std::string headers = SDPParser::extractHeaders(data);

        // Store caller RTP info
        SDPInfo caller = SDPParser::parse(sdp);
        session->rtp->caller_ip   = caller.ip;
        session->rtp->caller_port = caller.port;

        logMsg = "rtp caller ip " + session->rtp->caller_ip + " port " + std::to_string(session->rtp->caller_port)+"\n";
        std::cout << logMsg << std::endl;
        m_pDailyLog->WriteLog(kDebug, logMsg);

        if (session->rtp->server_port == 0)
            session->rtp->server_port = allocateRTPPort();

    
        

         // Build NEW INVITE (B2BUA)
        std::string newInvite = CreateNewInviteMsg(parser, session);

        //Send to callee
        m_pUDPSocket->send(newInvite.c_str(), newInvite.length(),
                           inet_addr(session->calleeIP.c_str()),
                           session->calleePort);

        logMsg = "NEW INVITE:\n" + newInvite;
        std::cout << logMsg << std::endl;
        m_pDailyLog->WriteLog(kDebug, logMsg);

        logMsg = "[ROUTING] Sending NEW INVITE to " + session->calleeIP + " " + std::to_string(session->calleePort);
        std::cout << logMsg << std::endl;
        m_pDailyLog->WriteLog(kDebug, logMsg);
       
    }
    catch (const std::exception& e)
    {
        m_pDailyLog->WriteLog(kGeneralError, "ERROR:processInviteMessage " + std::string(e.what()));
    }
}

// ===================================================================
// ACK 
// ===================================================================
void SIPServer::processAckMessage(const SIPParser& parser,
                                  const std::string &sipMsg,
                                  const std::string& addr_ip,
                                  uword port)
{
    try
    {   
        logMsg = "recevied ACK "+ addr_ip + ":" + std::to_string(port)+"\n"+sipMsg;
        std::cout << logMsg << std::endl;
        m_pDailyLog->WriteLog(kDebug, logMsg);

        std::string callID = parser.getCallID();
        auto session = m_callsessionManager->getSessionByCallerCallID(callID);
        if (!session) return;

        //build + send ACK to callee
        std::string ack = buildAckToCallee(parser, session);

        m_pUDPSocket->send(ack.c_str(), ack.length(),
                        inet_addr(session->calleeIP.c_str()),
                        session->calleePort);
         
        logMsg = "Sent ACK to callee "+ session->calleeIP + ":" + std::to_string(session->calleePort)+"\n"+ack;
        std::cout << logMsg << std::endl;
        m_pDailyLog->WriteLog(kDebug, logMsg);

    }
    catch (const std::exception& e) {
        m_pDailyLog->WriteLog(kGeneralError, "ERROR:processAckMessage " + std::string(e.what()));
    }
}

// ===================================================================
//  BYE 
// ===================================================================
void SIPServer::processByeMessage(const SIPParser& parser,
                                  const std::string &sipMsg,
                                  const std::string& addr_ip,
                                  uword port)
{
    try
    {   
        logMsg = "recevied BYE "+ addr_ip + ":" + std::to_string(port)+"\n"+sipMsg;
        std::cout << logMsg << std::endl;
        m_pDailyLog->WriteLog(kDebug, logMsg);

       std::string callID = parser.getCallID();

        // find session (caller side first)
        auto session = m_callsessionManager->getSessionByCallerCallID(callID);

        bool fromCaller = true;

        if (!session)
        {
            session = m_callsessionManager->getSessionByCalleeCallID(callID);
            fromCaller = false;
        }

        if (!session)
        {
            std::cout << "BYE: session not found\n";
            return;
        }

        //  Send 200 OK to sender
        std::string ok = build200OkForBye(parser);

        m_pUDPSocket->send(ok.c_str(), ok.length(),
                        inet_addr(addr_ip.c_str()), port);

        logMsg = "Send 200 ok bye msg  "+ addr_ip + ":" + std::to_string(port)+"\n"+ok;
        std::cout << logMsg << std::endl;
        m_pDailyLog->WriteLog(kDebug, logMsg);

        if(!session->isTerminated)
        {
            //  Send BYE to other side
            std::string bye = buildBye(session, fromCaller);

            std::string targetIP = fromCaller ? session->calleeIP : session->callerIP;
            int targetPort       = fromCaller ? session->calleePort : session->callerPort;

            m_pUDPSocket->send(bye.c_str(), bye.length(),
                            inet_addr(targetIP.c_str()), targetPort);

            logMsg = "Send  BYE msg  "+ targetIP + ":" + std::to_string(targetPort)+"\n"+bye;
            std::cout << logMsg << std::endl;
            m_pDailyLog->WriteLog(kDebug, logMsg);

            session->isTerminated = true;
            session->byeConfirmedCount++;
        }        

    }
    catch (const std::exception& e) {
        m_pDailyLog->WriteLog(kGeneralError, "ERROR:processByeMessage " + std::string(e.what()));
    }
}

// ===================================================================
//  CANCEL 
// ===================================================================
void SIPServer::processCancelMessage(const SIPParser& parser,
                                     const std::string &sipMsg,
                                     const std::string& addr_ip,
                                     uword port)
{
    try
    {
        std::string callID = parser.getCallID();
        CallSession* session = m_callsessionManager->getSession(callID);
        if (!session) return;

        if (addr_ip == session->callerIP && port == session->callerPort)
        {
            m_pUDPSocket->send(sipMsg.c_str(), sipMsg.length(),
                inet_addr(session->calleeIP.c_str()), session->calleePort);
            std::cout << "[CANCEL] Caller → Callee\n";
        }
        else
        {
            m_pUDPSocket->send(sipMsg.c_str(), sipMsg.length(),
                inet_addr(session->callerIP.c_str()), session->callerPort);
            std::cout << "[CANCEL] Callee → Caller\n";
        }

        session->state = "TERMINATED";
        m_callsessionManager->removeSession(callID);
    }
    catch (const std::exception &e)
    {
        m_pDailyLog->WriteLog(kGeneralError, "ERROR: SIPServer::build100Trying: " + std::string(e.what()));
    }
}

// ===================================================================
//  TRYING
// ===================================================================
std::string SIPServer::build100Trying(const std::shared_ptr<CallSession>& session)
{
    try{
        std::string response =
        "SIP/2.0 100 Trying\r\n"
        "Via: " + session->callerVia + "\r\n"
        "From: " + session->callerFrom + "\r\n"
        "To: " + session->callerTo + "\r\n"   // NO tag here
        "Call-ID: " + session->callerCallID + "\r\n"
        "CSeq: " + session->callerCSeq + "\r\n"
        "Content-Length: 0\r\n\r\n";

        return response;
    }
    catch (const std::exception &e)
    {
        m_pDailyLog->WriteLog(kGeneralError, "ERROR: SIPServer::build100Trying: " + std::string(e.what())); 
        return "";
    }
}

// ===================================================================
//  RINIGING
// ===================================================================
std::string SIPServer::buildRingingMsg(const std::shared_ptr<CallSession>& session)
{
    try{
        std::string to = removeAllTags(session->callerTo);
        to += ";tag=" + session->callertoTag;
        //session->callertoTag = generateTag();
        std::string response =
        "SIP/2.0 180 Ringing\r\n"
        "Via: " + session->callerVia + "\r\n"
        "From: " + session->callerFrom + "\r\n"
        "To: " + to + "\r\n"
        "Call-ID: " + session->callerCallID + "\r\n"
        "CSeq: " + session->callerCSeq + "\r\n"
        "Contact: <sip:" + m_sServer_ip + ":5060>\r\n"
        "Record-Route: <sip:" + m_sServer_ip + ":5060;lr>\r\n"
        "Content-Length: 0\r\n\r\n";

        return response;
    }
    catch (const std::exception &e)
    {
        m_pDailyLog->WriteLog(kGeneralError, "ERROR: SIPServer::buildRingingMsg: " + std::string(e.what())); 
        return "";
    }
}

// ===================================================================
//  200 OK 
// ===================================================================
std::string SIPServer::build200OkMsg(const std::shared_ptr<CallSession>& session)
{
    try{
        std::string to = removeAllTags(session->callerTo);
        to += ";tag=" + session->callertoTag;

        std::string response =
        "SIP/2.0 200 OK\r\n"
        "Via: " + session->callerVia + "\r\n"
        "From: " + session->callerFrom + "\r\n"
        "To: " + to + "\r\n"
        "Call-ID: " + session->callerCallID + "\r\n"
        "CSeq: " + session->callerCSeq + "\r\n"
        "Contact: <sip:" + m_sServer_ip + ":5060>\r\n"
        "Content-Type: application/sdp\r\n"
        "Content-Length: " + std::to_string(session->callersdp.length()) + "\r\n\r\n" +
        session->callersdp;

        return response;
    }
    catch (const std::exception &e)
    {
        m_pDailyLog->WriteLog(kGeneralError, "ERROR: SIPServer::build200OkMsg: " + std::string(e.what())); 
        return "";
    }
}
/* for testing */
void SIPServer::debug_testing()
{
    try
    {
        
        std::string data1 =
        "REGISTER sip:192.168.1.7 SIP/2.0\r\n"
        "Via: SIP/2.0/UDP 192.168.1.2:59986;branch=z9hG4bK-reg1001;rport\r\n"
        "Max-Forwards: 70\r\n"
        "Contact: <sip:1001@192.168.1.2:59986>\r\n"
        "To: <sip:1001@192.168.1.7>\r\n"
        "From: <sip:1001@192.168.1.7>;tag=tag1001\r\n"
        "Call-ID: reg1001@192.168.1.2\r\n"
        "CSeq: 1 REGISTER\r\n"
        "Expires: 3600\r\n"
        "User-Agent: TestClient-1001\r\n"
        "Content-Length: 0\r\n"
        "\r\n";

        std::string data2 =
        "REGISTER sip:192.168.1.7 SIP/2.0\r\n"
        "Via: SIP/2.0/UDP 192.168.1.3:57012;branch=z9hG4bK-reg1002;rport\r\n"
        "Max-Forwards: 70\r\n"
        "Contact: <sip:1002@192.168.1.3:57012>\r\n"
        "To: <sip:1002@192.168.1.7>\r\n"
        "From: <sip:1002@192.168.1.7>;tag=tag1002\r\n"
        "Call-ID: reg1002@192.168.1.3\r\n"
        "CSeq: 1 REGISTER\r\n"
        "Expires: 3600\r\n"
        "User-Agent: TestClient-1002\r\n"
        "Content-Length: 0\r\n"
        "\r\n";


        std::string data3 =
        "INVITE sip:1001@192.168.1.7;transport=UDP SIP/2.0\r\n"
        "Via: SIP/2.0/UDP 192.168.1.3:57012;branch=z9hG4bK-524287-1---a49e592b77391880;rport\r\n"
        "Max-Forwards: 70\r\n"
        "Contact: <sip:1002@192.168.1.3:57012;transport=UDP>\r\n"
        "To: <sip:1001@192.168.1.7>\r\n"
        "From: <sip:1002@192.168.1.7;transport=UDP>;tag=11f61c03\r\n"
        "Call-ID: F3c48OkEFhowhfeZNK4C7A..\r\n"
        "CSeq: 1 INVITE\r\n"
        "Allow: INVITE, ACK, CANCEL, BYE, NOTIFY, REFER, MESSAGE, OPTIONS, INFO, SUBSCRIBE\r\n"
        "Content-Type: application/sdp\r\n"
        "Supported: replaces, norefersub, extended-refer, timer, sec-agree, outbound, path, X-cisco-serviceuri\r\n"
        "User-Agent: Zoiper v2.10.20.11\r\n"
        "Allow-Events: presence, kpml, talk, as-feature-event\r\n"
        "Content-Length: 194\r\n"
        "\r\n"
        "v=0\r\n"
        "o=Zoiper 0 216745852 IN IP4 192.168.1.3\r\n"
        "s=Zoiper\r\n"
        "c=IN IP4 192.168.1.3\r\n"
        "t=0 0\r\n"
        "m=audio 37392 RTP/AVP 0 101 8 3\r\n"
        "a=rtpmap:101 telephone-event/8000\r\n"
        "a=fmtp:101 0-16\r\n"
        "a=sendrecv\r\n"
        "a=rtcp-mux\r\n";

        std::string data4 =
        "SIP/2.0 100 Trying\r\n"
        "Via: SIP/2.0/UDP 192.168.1.7:5060;branch=z9hG4bK-18a4d8a918bd44cabebff80bee5f5cc1;rport=5060\r\n"
        "To: <sip:1001@192.168.1.2>\r\n"
        "From: <sip:1002@192.168.1.7;transport=UDP>;tag=6QBEY032wn\r\n"
        "Call-ID: 18a4ecbad391a65277ca95695340b8c2@192.168.1.7\r\n"
        "CSeq: 1 INVITE\r\n"
        "Content-Length: 0\r\n"
        "\r\n";

        std::string data5 =
        "SIP/2.0 180 Ringing\r\n"
        "Via: SIP/2.0/UDP 192.168.1.7:5060;branch=z9hG4bK-18a4d8a918bd44cabebff80bee5f5cc1;rport=5060\r\n"
        "Record-Route: <sip:192.168.1.7:5060;lr>\r\n"
        "Contact: <sip:1001@192.168.1.2:59986>\r\n"
        "To: <sip:1001@192.168.1.2>;tag=e87dd871\r\n"
        "From: <sip:1002@192.168.1.7;transport=UDP>;tag=6QBEY032wn\r\n"
        "Call-ID: 18a4ecbad391a65277ca95695340b8c2@192.168.1.7\r\n"
        "CSeq: 1 INVITE\r\n"
        "Allow: INVITE, ACK, CANCEL, BYE, NOTIFY, REFER, MESSAGE, OPTIONS, INFO, SUBSCRIBE\r\n"
        "Supported: replaces, norefersub, extended-refer, timer, sec-agree, outbound, path, X-cisco-serviceuri\r\n"
        "User-Agent: Zoiper v2.10.20.11\r\n"
        "Allow-Events: presence, kpml, talk, as-feature-event\r\n"
        "Content-Length: 0\r\n"
        "\r\n";

        std::string data6 =
        "SIP/2.0 200 OK\r\n"
        "Via: SIP/2.0/UDP 192.168.1.7:5060;branch=z9hG4bK-18a4d8a918bd44cabebff80bee5f5cc1;rport=5060\r\n"
        "Record-Route: <sip:192.168.1.7:5060;lr>\r\n"
        "Contact: <sip:1001@192.168.1.2:59986>\r\n"
        "To: <sip:1001@192.168.1.2>;tag=e87dd871\r\n"
        "From: <sip:1002@192.168.1.7;transport=UDP>;tag=6QBEY032wn\r\n"
        "Call-ID: 18a4ecbad391a65277ca95695340b8c2@192.168.1.7\r\n"
        "CSeq: 1 INVITE\r\n"
        "Allow: INVITE, ACK, CANCEL, BYE, NOTIFY, REFER, MESSAGE, OPTIONS, INFO, SUBSCRIBE\r\n"
        "Content-Type: application/sdp\r\n"
        "Supported: replaces, norefersub, extended-refer, timer, sec-agree, outbound, path, X-cisco-serviceuri\r\n"
        "User-Agent: Zoiper v2.10.20.11\r\n"
        "Allow-Events: presence, kpml, talk, as-feature-event\r\n"
        "Content-Length: 194\r\n"
        "\r\n"
        "v=0\r\n"
        "o=Zoiper 0 153307809 IN IP4 192.168.1.2\r\n"
        "s=Zoiper\r\n"
        "c=IN IP4 192.168.1.2\r\n"
        "t=0 0\r\n"
        "m=audio 53109 RTP/AVP 0 8 3 101\r\n"
        "a=rtpmap:101 telephone-event/8000\r\n"
        "a=fmtp:101 0-16\r\n"
        "a=sendrecv\r\n"
        "a=rtcp-mux\r\n";


        std::string data7 =
        "ACK sip:1001@192.168.1.7;transport=UDP SIP/2.0\r\n"
        "Via: SIP/2.0/UDP 192.168.1.3:57012;branch=z9hG4bK-ack524287;rport\r\n"
        "Max-Forwards: 70\r\n"
        "Contact: <sip:1002@192.168.1.3:57012;transport=UDP>\r\n"
        "To: <sip:1001@192.168.1.7>;tag=e87dd871\r\n"
        "From: <sip:1002@192.168.1.7;transport=UDP>;tag=11f61c03\r\n"
        "Call-ID: F3c48OkEFhowhfeZNK4C7A..\r\n"
        "CSeq: 1 ACK\r\n"
        "Content-Length: 0\r\n"
        "\r\n";

        std::string data8 =
        "BYE sip:1001@192.168.1.7;transport=UDP SIP/2.0\r\n"
        "Via: SIP/2.0/UDP 192.168.1.3:57012;branch=z9hG4bK-bye524287;rport\r\n"
        "Max-Forwards: 70\r\n"
        "Contact: <sip:1002@192.168.1.3:57012;transport=UDP>\r\n"
        "To: <sip:1001@192.168.1.7>;tag=e87dd871\r\n"
        "From: <sip:1002@192.168.1.7;transport=UDP>;tag=11f61c03\r\n"
        "Call-ID: F3c48OkEFhowhfeZNK4C7A..\r\n"
        "CSeq: 2 BYE\r\n"
        "Content-Length: 0\r\n"
        "\r\n";

        // std::string data9 =
        // "BYE sip:1002@192.168.1.7;transport=UDP SIP/2.0\r\n"
        // "Via: SIP/2.0/UDP 192.168.1.2:59986;branch=z9hG4bK-bye987654;rport\r\n"
        // "Max-Forwards: 70\r\n"
        // "Contact: <sip:1001@192.168.1.2:59986>\r\n"
        // "To: <sip:1002@192.168.1.7;transport=UDP>;tag=6QBEY032wn\r\n"
        // "From: <sip:1001@192.168.1.2>;tag=e87dd871\r\n"
        // "Call-ID: 18a4ecbad391a65277ca95695340b8c2@192.168.1.7\r\n"
        // "CSeq: 2 BYE\r\n"
        // "Content-Length: 0\r\n"
        // "\r\n";

        std::string data9 =
        "SIP/2.0 200 OK\r\n"
        "Via: SIP/2.0/UDP 192.168.1.7:5060;branch=z9hG4bK-18a4edbe5e91d766c70ab1f45e5ded2b;rport\r\n"
        "From: <sip:1002@192.168.1.7;transport=UDP>;tag=11f61c03\r\n"
        "To: <sip:1001@192.168.1.2>;tag=e87dd871\r\n"
        "Call-ID: 18a4ecbad391a65277ca95695340b8c2@192.168.1.7\r\n"
        "CSeq: 2 BYE\r\n"
        "Content-Length: 0\r\n"
        "\r\n";

        // Simulated client IP & port
        std::string testIP1 = "192.168.1.2";
        std::string testIP2 = "192.168.1.3";
        //uword testPort = 5060;
        uword testPort1 = 59986;
        uword testPort2 = 57012;
        processSipMessage(data1, testIP1, testPort1);
        processSipMessage(data2, testIP2, testPort2);
        processSipMessage(data3, testIP2, testPort2);
        processSipMessage(data4, testIP1, testPort1);
        processSipMessage(data5, testIP1, testPort1);
        processSipMessage(data6, testIP1, testPort1);
        processSipMessage(data7, testIP2, testPort2);
        processSipMessage(data8, testIP2, testPort2);
        processSipMessage(data9, testIP1, testPort1);
    }
    catch (const std::exception &e)
    {
        m_pDailyLog->WriteLog(kGeneralError, "ERROR: SIPServer::debug_testing: " + std::string(e.what())); 
    }
}

/* Start RTPRelay */ 
// void SIPServer::startRTPRelay(RTPSession &session) {
//     try{

//         std::thread(rtpRelayWorker, std::ref(session)).detach();
//         //std::thread(rtpRelayWorker, std::ref(session), m_pDailyLog).detach();
//         m_pDailyLog->WriteLog(kGeneralError, ":startRTPRelay: "); 
//     }
//     catch (const std::exception &e)
//     {
//         m_pDailyLog->WriteLog(kGeneralError, "ERROR: SIPServer::startRTPRelay: " + std::string(e.what())); 
//     }
    
// }
// void SIPServer::startRTPRelay(std::shared_ptr<RTPSession> session) {
//     try {
//         // Capture the shared_ptr by value to increment the reference count
//         std::thread([session]() {
//             rtpRelayWorker(session); 
//         }).detach();
        
//         m_pDailyLog->WriteLog(kGeneralError, ":startRTPRelay: Thread Launched"); 
//     }
//     catch (const std::exception &e) {
//         m_pDailyLog->WriteLog(kGeneralError, "ERROR: " + std::string(e.what())); 
//     }
// }
// /* Stop RTPRelay */ 
// void SIPServer::stopRTPRelay(RTPSession& rtp)
// {
    
//     try{

//         rtp.running = false;

//         if (rtp.sockfd > 0)
//         {
//             close(rtp.sockfd);   //  force unblock recvfrom
//             std::cout << "[RTP] Stopped\n";
//         }
//     }
//     catch (const std::exception &e)
//     {
//        m_pDailyLog->WriteLog(kGeneralError, "ERROR: SIPServer::StopRTPRelay: " + std::string(e.what())); 
//     }

// } 

void SIPServer::startRTPRelay(std::shared_ptr<RTPSession> rtp)
{
    try
    {
        if (!rtp) return;

        std::thread([rtp]() {
            rtpRelayWorker(rtp);
        }).detach();

        m_pDailyLog->WriteLog(kDebug, "[RTP] Relay thread started");
    }
    catch (const std::exception &e)
    {
        m_pDailyLog->WriteLog(kGeneralError, "ERROR startRTPRelay: " + std::string(e.what()));
    }
}
void SIPServer::stopRTPRelay(std::shared_ptr<RTPSession> rtp)
{
    try
    {
        if (!rtp) return;

        rtp->running = false;

        if (rtp->sockfd > 0)
        {
            close(rtp->sockfd); // unblock recv
            rtp->sockfd = -1;
        }

        std::cout << "[RTP] Relay stopped\n";
    }
    catch (const std::exception &e)
    {
        m_pDailyLog->WriteLog(kGeneralError, "ERROR stopRTPRelay: " + std::string(e.what()));
    }
}

/* Clean up Registrations*/
void SIPServer::cleanupRegistrations()
{
    
    try{

        time_t now = time(nullptr);

        for (auto it = m_registrationDB.begin(); it != m_registrationDB.end(); )
        {
            if (now > it->second.expiry)
            {
                std::cout << "[REGISTER] Expired: " << it->first << std::endl;
                it = m_registrationDB.erase(it);
            }
            else
            {
                ++it;
            }
        }
    }
    catch (const std::exception &e)
    {
        m_pDailyLog->WriteLog(kGeneralError, "ERROR: SIPServer::cleanupRegistrations: " + std::string(e.what())); 
    }
}

/* Create New Invite Msg */
std::string SIPServer::CreateNewInviteMsg(const SIPParser& parser,
                                           const std::shared_ptr<CallSession>& session)
{   
    try{

        //  NEW Call-ID 
        std::string newCallID = session->calleeCallID;

        //  NEW From tag
        std::string from = parser.getFrom();
        from = removeAllTags(from);
        session->calleefromTag = generateTag();
        from += ";tag=" +  session->calleefromTag;

        std::string to = "<sip:"+parser.getToUser() + "@" +session->calleeIP+">";

        session->calleeFrom = from;
        session->calleeTo = to;
        session->calleeCSeq = 1;
        session->CSeq= 1;

        //  Request line (send to callee)
        std::string requestLine = "INVITE sip:" + parser.getToUser() + "@" +
                                session->calleeIP + " SIP/2.0\r\n";

        session->calleeBranch = generateBranch();                     
        //  Via 
        std::string via =
            "Via: SIP/2.0/UDP " + m_sServer_ip +
            ":5060;branch=" + session->calleeBranch + ";rport\r\n";

        //  Contact (your proxy)
        std::string contact =
            "Contact: <sip:" + m_sServer_ip + ":5060>\r\n";

        std::string Record_Route = "Record-Route: <sip:"+m_sServer_ip+":5060;lr>\r\n";
        //  Max-Forwards
        std::string maxForwards = "Max-Forwards: 70\r\n";

        
        //  Headers
        std::string headers =
            via +
            "From: " + from + "\r\n" +
            "To: " + to + "\r\n" +
            "Call-ID: " + newCallID + "\r\n" +
            "CSeq: 1 INVITE\r\n" +
            maxForwards +
            contact +
            Record_Route+
            "Content-Type: application/sdp\r\n";

        //  Modify SDP (RTP relay ready)
        SDPInfo caller = SDPParser::parse(session->callersdp);

        std::string newSDP =
            "v=0\r\n"
            "o=proxy 0 0 IN IP4 " + m_sServer_ip + "\r\n"
            "s=VoIP Call\r\n"
            "c=IN IP4 " + m_sServer_ip + "\r\n"
            "t=0 0\r\n"
            "m=audio " + std::to_string(session->rtp->server_port) + " RTP/AVP 0 101 8 3\r\n"
            "a=rtpmap:101 telephone-event/8000\r\n"
            "a=sendrecv\r\n";

        //  Content-Length
        headers += "Content-Length: " + std::to_string(newSDP.length()) + "\r\n\r\n";

        std::string finalMsg = requestLine + headers + newSDP;

        finalMsg = SDPParser::updateContentLength(finalMsg, newSDP.length());

        session->callersdp = newSDP;

        //  Final message
        return finalMsg;

    }catch (const std::exception& e)
    {
        m_pDailyLog->WriteLog(kGeneralError, "ERROR: SIPServer::CreateNewInviteMsg: " + std::string(e.what())); 
        return "";
    }

    
}

// std::string SIPServer::CreateNewInviteMsg(const SIPParser& parser,
//                                            const std::shared_ptr<CallSession>& session)
// {   
//     try {
//         // 1. Setup Identities for Leg B
//         std::string newCallID = session->calleeCallID;

//         // Strip tags from original From and add our own Server-generated tag for the Callee side
//         std::string from = parser.getFrom();
//         from = removeAllTags(from); 
//         session->calleefromTag = generateTag(); 
//         from += ";tag=" + session->calleefromTag;

//         // The 'To' should point to the registered callee destination
//         std::string to = "<sip:" + parser.getToUser() + "@" + session->calleeIP + ">";

//         session->calleeFrom = from;
//         session->calleeTo   = to;
//         session->calleeCSeq = 1;
//         session->CSeq       = 1;

//         // 2. Build Headers
//         std::string requestLine = "INVITE sip:" + parser.getToUser() + "@" + session->calleeIP + " SIP/2.0\r\n";
//         session->calleeBranch   = generateBranch();                     
        
//         std::string via = "Via: SIP/2.0/UDP " + m_sServer_ip + ":5060;branch=" + session->calleeBranch + ";rport\r\n";
//         std::string contact = "Contact: <sip:" + m_sServer_ip + ":5060>\r\n";
//         std::string recordRoute = "Record-Route: <sip:" + m_sServer_ip + ":5060;lr>\r\n";

//         std::string headers =
//             via +
//             "From: " + from + "\r\n" +
//             "To: " + to + "\r\n" +
//             "Call-ID: " + newCallID + "\r\n" +
//             "CSeq: 1 INVITE\r\n" +
//             "Max-Forwards: 70\r\n" +
//             contact +
//             recordRoute +
//             "Content-Type: application/sdp\r\n";

//         // 3. Build the Relay SDP
//         // This tells the Callee: "Send your audio to my server_port"
//         std::string newSDP =
//             "v=0\r\n"
//             "o=proxy 123456 123456 IN IP4 " + m_sServer_ip + "\r\n"
//             "s=SIP Call\r\n"
//             "c=IN IP4 " + m_sServer_ip + "\r\n"
//             "t=0 0\r\n"
//             "m=audio " + std::to_string(session->rtp.server_port) + " RTP/AVP 0 8 101\r\n"
//             "a=rtpmap:0 PCMU/8000\r\n"
//             "a=rtpmap:8 PCMA/8000\r\n"
//             "a=rtpmap:101 telephone-event/8000\r\n"
//             "a=fmtp:101 0-16\r\n"
//             "a=sendrecv\r\n";

//         headers += "Content-Length: " + std::to_string(newSDP.length()) + "\r\n\r\n";

//         // IMPORTANT: DO NOT do "session->callersdp = newSDP;"
//         // We need session->callersdp to remain the REAL IP of the caller 
//         // so the RTP worker knows where to forward the packets!

//         return requestLine + headers + newSDP;

//     } catch (const std::exception& e) {
//         m_pDailyLog->WriteLog(kGeneralError, "ERROR: SIPServer::CreateNewInviteMsg: " + std::string(e.what())); 
//         return "";
//     }
// }

/* generate CallID*/
std::string SIPServer::generateCallID()
{   
    try{

        auto now = std::chrono::high_resolution_clock::now().time_since_epoch().count();

        std::random_device rd;
        std::mt19937_64 gen(rd());
        std::uniform_int_distribution<uint64_t> dis;

        uint64_t randomPart = dis(gen);

        std::stringstream ss;
        ss << std::hex << now << randomPart << "@" << m_sServer_ip;

        return ss.str();
    }
    catch (const std::exception& e)
    {
        m_pDailyLog->WriteLog(kGeneralError, "ERROR: SIPServer::generateCallID: " + std::string(e.what())); 
        return "";
    }
    
}

/* generate Tag */
std::string SIPServer::generateTag()
{   
    try{

        static const char alphanum[] =
        "0123456789"
        "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
        "abcdefghijklmnopqrstuvwxyz";

        std::string tag;
        int length = 10;

        std::random_device rd;
        std::mt19937 gen(rd());
        std::uniform_int_distribution<> dis(0, sizeof(alphanum) - 2);

        for (int i = 0; i < length; ++i)
            tag += alphanum[dis(gen)];

        return tag;
    }
    catch (const std::exception& e)
    {
        m_pDailyLog->WriteLog(kGeneralError, "ERROR: SIPServer::generateTag: " + std::string(e.what())); 
        return "";
    }
    
}

/* generate Branch*/
std::string SIPServer::generateBranch()
{   
    try{

        std::stringstream ss;
        ss << "z9hG4bK-";

        auto now = std::chrono::high_resolution_clock::now().time_since_epoch().count();

        std::random_device rd;
        std::mt19937_64 gen(rd());
        std::uniform_int_distribution<uint64_t> dis;

        ss << std::hex << now << dis(gen);

        return ss.str();
    }
    catch (const std::exception& e)
    {
        m_pDailyLog->WriteLog(kGeneralError, "ERROR: SIPServer::generateBranch: " + std::string(e.what())); 
        return "";
    }
    
}

std::string SIPServer::removeAllTags(const std::string& header)
{
    
    try{

        std::string result = header;

        size_t pos;
        while ((pos = result.find(";tag=")) != std::string::npos)
        {
            size_t end = result.find(";", pos + 1);
            size_t lineEnd = result.find("\r\n", pos);

            if (end == std::string::npos || (lineEnd != std::string::npos && lineEnd < end))
                end = lineEnd;

            result.erase(pos, end - pos);
        }

        return result;
    }
    catch (const std::exception& e)
    {
        m_pDailyLog->WriteLog(kGeneralError, "ERROR: SIPServer::removeAllTag: " + std::string(e.what())); 
        return "";
    }
}

// /* build BYE */
// std::string SIPServer::buildBye(const std::shared_ptr<CallSession>& session,
//                                 bool toCallee)
// {
//     try
//     {
//         std::string targetIP;
//         std::string callID;
//         std::string from;
//         std::string to;
//         std::string requestURI;
//         std::string toTag;

//         if (toCallee)
//         {
//             // Caller → Callee
//             targetIP   = session->calleeIP;
//             callID     = session->calleeCallID;
//             from       = session->calleeFrom;
//             to         = session->calleeTo;
//             requestURI = "sip:" + session->calleeUser + "@" + session->calleeIP;
//             toTag      = session->calleetoTag;
//         }
//         else
//         {
//             // Callee → Caller   IMPORTANT FIX
//             targetIP   = session->callerIP;
//             callID     = session->callerCallID;
//             from       = session->callerFrom;
//             to         = session->callerTo;
//             requestURI = "sip:" + session->callerUser + "@" + session->callerIP;
//             toTag      = session->callertoTag;
//         }

//         std::string branch = generateBranch();

//         std::string bye =
//             "BYE " + requestURI + " SIP/2.0\r\n"
//             "Via: SIP/2.0/UDP " + m_sServer_ip + ":5060;branch=" + branch + ";rport\r\n"
//             "Max-Forwards: 70\r\n"
//             "From: " + from + "\r\n"
//             "To: " + to +";tag="+toTag + "\r\n"
//             "Call-ID: " + callID + "\r\n"
//             "CSeq: " + std::to_string(++session->CSeq) + " BYE\r\n"
//             "Content-Length: 0\r\n\r\n";

//         return bye;
//     }
//     catch (const std::exception &e)
//     {
//         m_pDailyLog->WriteLog(kGeneralError, "ERROR: SIPServer::buildBye: " + std::string(e.what())); 
//         return "";
//     }
// }

std::string SIPServer::buildBye(const std::shared_ptr<CallSession>& session, bool toCallee)
{
    std::string requestURI, fromHeader, toHeader, callID;

    if (toCallee) {
        // We are talking to Leg B (Callee)
        requestURI = "sip:" + session->calleeUser + "@" + session->calleeIP;
        callID     = session->calleeCallID;
        
        // From: Server (using server's tag for callee)
        // To: Callee (using callee's original tag)
        fromHeader = session->calleeFrom; 
        if (fromHeader.find("tag=") == std::string::npos) fromHeader += ";tag=" + session->serverTagForCallee;
        
        toHeader   = session->calleeTo;
        if (toHeader.find("tag=") == std::string::npos) toHeader += ";tag=" + session->calleetoTag;
    } 
    else {
        // We are talking to Leg A (Caller)
        requestURI = "sip:" + session->callerUser + "@" + session->callerIP;
        callID     = session->callerCallID;

        // SWAP FOR CALLER:
        // From: Should look like the Callee to the Caller
        fromHeader = session->callerTo; 
        if (fromHeader.find("tag=") == std::string::npos) fromHeader += ";tag=" + session->callertoTag; 
        
        // To: The Caller's identity
        toHeader   = session->callerFrom; 
    }

    std::string branch = generateBranch();
    return "BYE " + requestURI + " SIP/2.0\r\n"
           "Via: SIP/2.0/UDP " + m_sServer_ip + ":5060;branch=" + branch + ";rport\r\n"
           "Max-Forwards: 70\r\n"
           "From: " + fromHeader + "\r\n"
           "To: " + toHeader + "\r\n"
           "Call-ID: " + callID + "\r\n"
           "CSeq: " + std::to_string(++session->CSeq) + " BYE\r\n"
           "Content-Length: 0\r\n\r\n";
}

// ===================================================================
//  200 OK BYE 
// ===================================================================
std::string SIPServer::build200OkForBye(const SIPParser& parser)
{
    try
    {
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
        m_pDailyLog->WriteLog(kGeneralError, "ERROR: SIPServer::build200OkForBye: " + std::string(e.what())); 
        return "";
    }
}

