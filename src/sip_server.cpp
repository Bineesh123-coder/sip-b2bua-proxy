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
        m_sDataPath ="/opt/app/DATA";
        m_debugLevel = 40;
        m_log =1;
        
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
        }
        m_pUDPSocket = nullptr;

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

                    std::string sipMsg(buffer);

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
            m_pDailyLog = new Clogger(m_sDataPath, m_debugLevel, m_log, "Sip_Server");
            printf("CREATING LOG FILE\n");
            m_pDailyLog->CreateLog();
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

void SIPServer::processSipMessage(const std::string& sipMsg, const std::string& addr_ip, uword port)
{
    try
    {   
        SIPParser parser;
        parser.parse(sipMsg);

        //m_pDailyLog->WriteLog(kDebug,sipMsg);
        //std::cout<<sipMsg<<std::endl;

        std::string method = parser.getMethod();

        std::string firstLine = parser.getRequestLine();
        
        if (firstLine.find("SIP/2.0") == 0)
        {
            std::string callID = parser.getCallID();

            CallSession* session = m_sessionManager.getSession(callID);

            if (!session)
            {
                std::cout << "[WARN] Session not found for response\n";
                m_pDailyLog->WriteLog(kDebug,"[WARN] Session not found for response\n");
                return;
            }
            
            std::string cseqm =  parser.getCSeqMethod();
            logMsg = "[CALL " + callID + "] "+firstLine+ " ("+cseqm +") Received from CALLEE " +
                     addr_ip + ":" + std::to_string(port);
            std::cout << logMsg << std::endl;
            m_pDailyLog->WriteLog(kDebug, logMsg);

            //  IMPORTANT: Check direction
            if (session->isCaller(addr_ip,port))
            {
                // Response from caller → send to callee
                m_pUDPSocket->send(sipMsg.c_str(), sipMsg.length(),
                    inet_addr(session->targetIP.c_str()),
                    session->targetPort);

                logMsg = "[CALL " + callID + "] Response forwarded to CALLEE " +
                     session->targetIP + ":" + std::to_string(session->targetPort);
                std::cout << logMsg << std::endl;
                m_pDailyLog->WriteLog(kDebug, logMsg);
            }
            else
            {
                // Response from callee → send to caller
                m_pUDPSocket->send(sipMsg.c_str(), sipMsg.length(),
                    inet_addr(session->callerIP.c_str()),
                    session->callerPort);

                logMsg = "[CALL " + callID + "] Response forwarded to CALLER " +
                     session->callerIP + ":" + std::to_string(session->callerPort);
                std::cout << logMsg << std::endl;
                m_pDailyLog->WriteLog(kDebug, logMsg);
            }

            std::string statusLine = parser.getRequestLine();

            if (statusLine.find("180") != std::string::npos)
            {
                session->state = "RINGING";
                logMsg = "[CALL " + callID + "] State changed to " + session->state;
                std::cout << logMsg << std::endl;
                m_pDailyLog->WriteLog(kDebug, logMsg);
            }
            else if (statusLine.find("200") != std::string::npos && cseqm.find("BYE") != std::string::npos)
            {   
                session->state = "TERMINATED";
                logMsg = "[CALL " + callID + "] State changed to " + session->state;
                std::cout << logMsg << std::endl;
                m_pDailyLog->WriteLog(kDebug, logMsg);
                m_sessionManager.removeSession(callID);
                
                logMsg  = "[CALL " + callID + "] Session deleted after BYE 200 OK";
                std::cout << logMsg << std::endl;
                m_pDailyLog->WriteLog(kDebug, logMsg);
            }
            else if (statusLine.find("200") != std::string::npos && cseqm.find("CANCEL") != std::string::npos)
            {   
                session->state = "CANCELLING";
                logMsg = "[CALL " + callID + "] State changed to " + session->state;
                std::cout << logMsg << std::endl;
                m_pDailyLog->WriteLog(kDebug, logMsg);
            }
            else if (statusLine.find("200") != std::string::npos)
            {
                session->state = "CONNECTED";
                logMsg = "[CALL " + callID + "] State changed to " + session->state;
                std::cout << logMsg << std::endl;
                m_pDailyLog->WriteLog(kDebug, logMsg);
            }
            else if (statusLine.find("487") != std::string::npos)
            {
                session->state = "TERMINATED";
                logMsg = "[CALL " + callID + "] State changed to " + session->state;
                std::cout << logMsg << std::endl;
                m_pDailyLog->WriteLog(kDebug, logMsg);
                m_sessionManager.removeSession(callID);

                logMsg  = "[CALL " + callID + "] Session deleted after 487 Terminated msg";
                std::cout << logMsg << std::endl;
                m_pDailyLog->WriteLog(kDebug, logMsg);
                
            }
        }

        if (method == "INVITE")
        {
            std::string Via = parser.getVia();
            std::string callID = parser.getCallID();
            std::string from   = parser.getFrom();
            std::string to     = parser.getTo();
            std::string cSeq     = parser.getCSeq();

            // std::cout << "INVITE Received\n";
            // std::cout << "Via: " << Via << std::endl;
            // std::cout << "Call-ID: " << callID << std::endl;
            // std::cout << "From: " << from << std::endl;
            // std::cout << "To: " << to << std::endl;
            // std::cout << "cSeq: " << cSeq << std::endl;

            // std::string logMsg =
            //     "INVITE Received | Via: " + Via +
            //     " | Call-ID: " + callID +
            //     " | From: " + from +
            //     " | To: " + to +
            //     " | CSeq: " + cSeq;

            // m_pDailyLog->WriteLog(kDebug, logMsg);

            processInviteMessage(parser,sipMsg, addr_ip, port);
        }
        else if (method == "ACK")
        {
            processAckMessage(parser,sipMsg,addr_ip,port);
        }
        else if (method == "BYE")
        {
            processByeMessage(parser,sipMsg,addr_ip,port);
        }
        else if (method == "CANCEL")
        {
            processCancelMessage(parser,sipMsg,addr_ip,port);
        }
        
    }
    catch (const std::exception& e)
    {
        m_pDailyLog->WriteLog(kGeneralError,
            std::string("Exception: processSipMessage: ") + e.what());
    }
    catch (...)
    {
        m_pDailyLog->WriteLog(kGeneralError,
            "Unknown Exception: processSipMessage");
    }
}

void SIPServer::processInviteMessage(const SIPParser& parser,const std::string &data, const std::string &ip, uword port)
{   
    try{
        
        
        logMsg = "INVITE received from "+ ip+":"+ std::to_string(port);
        std::cout<<logMsg<<std::endl;
        m_pDailyLog->WriteLog(kDebug, logMsg);

        // Send proper 100 Trying
        std::string trying = build100Trying(parser);

        std::string targetIP = "127.0.0.1";
        int targetPort = 5061;   // ✅ FIX

        

        udword clientIP = inet_addr(ip.c_str());
        m_pUDPSocket->send(trying.c_str(), trying.length(), clientIP, port);
        //m_pDailyLog->WriteLog(kDebug,trying.c_str());

        logMsg = "Sending 100 Trying message to "+ ip+":"+ std::to_string(port);
        std::cout<<logMsg<<std::endl;
        m_pDailyLog->WriteLog(kDebug, logMsg);

        // STEP 2: Extract target from Request-URI
        std::string requestLine = parser.getRequestLine();

        // std::string targetIP;
        // int targetPort;

        

        //parser.parseRequestURI(requestLine, targetIP, targetPort);

        
        CallSession session;
        session.callID = parser.getCallID();
        session.callerIP = ip;
        session.callerPort = port;

        session.targetIP = targetIP;
        session.targetPort = targetPort;

        session.state = "INVITE_SENT";

        //  store
        m_sessionManager.addSession(session);

        logMsg = "[CALL " + session.callID + "] Session Created Caller ("+session.callerIP+":"+std::to_string(session.callerPort)+")";
        std::cout << logMsg << std::endl;
        m_pDailyLog->WriteLog(kDebug, logMsg);
        
        m_pUDPSocket->send(data.c_str(), data.length(),
                        inet_addr(targetIP.c_str()), targetPort);

        
        logMsg ="[CALL " + session.callID + "] INVITE forwarded to " + targetIP + ":" + std::to_string(targetPort);
        std::cout<<logMsg<<std::endl;
        m_pDailyLog->WriteLog(kDebug, logMsg);

        
    }
    catch (const std::exception& e)
    {
        m_pDailyLog->WriteLog(kGeneralError,
            "ERROR:processInviteMessage" + std::string(e.what()));
    }
    
}

void SIPServer::processAckMessage(const SIPParser& parser,
                                 const std::string &sipMsg,
                                 const std::string& addr_ip,
                                 uword port)
{
    try {
        std::string callID = parser.getCallID();
        
        CallSession* session = m_sessionManager.getSession(callID);

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

        CallSession* session = m_sessionManager.getSession(callID);

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

        CallSession* session = m_sessionManager.getSession(callID);

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


// void SIPServer::processOptionsMessage(const std::string& data, const std::string& ip)
// {
//     std::cout << "Processing OPTIONS from " << ip << std::endl;

//     std::string response =
//     "SIP/2.0 200 OK\r\n"
//     "Allow: INVITE, ACK, BYE, OPTIONS\r\n"
//     "Content-Length: 0\r\n\r\n";

//     // send response
// }

std::string SIPServer::build100Trying(const SIPParser& parser)
{
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

std::string SIPServer::build_200_OK(const SIPParser& parser)
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

            
           