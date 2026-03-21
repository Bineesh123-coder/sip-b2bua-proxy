#include "sip_server.h"


SIPServer::SIPServer()
{
    try{
        memset(m_logString, 0, 2048);
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

                    std::cout << "\n=== NEW SIP MESSAGE ===\n";
                    std::cout << buffer << std::endl;

                    // Convert IP to readable format
                    struct in_addr addr;
                    addr.s_addr = ip;

                    std::cout << "From IP: " << inet_ntoa(addr)
                            << " Port: " << port << std::endl;
                }
            
           
            // Sleep for a short duration to prevent high CPU usage
            //std::this_thread::sleep_for(std::chrono::milliseconds(100)); // Adjust sleep time as needed

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
        m_pDailyLog->WriteLog(kGeneralError, "CSynwayRecorder::stop()::Begin()");
        if (m_bStarted)
        {
            m_bStarted = false;
            m_bStopped = true;

        
            std::this_thread::sleep_for(std::chrono::milliseconds(200));
           

        }
    }
    catch (...)
    {
        m_pDailyLog->WriteLog(kFatalError, "CSynwayRecorder::stop()::Exception!");
        return kFailure;
    }
    return kSuccess;
}

