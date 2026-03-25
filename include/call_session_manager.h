#ifndef CALL_SESSION_MANAGER_H
#define CALL_SESSION_MANAGER_H

#include <map>
#include <string>
#include <mutex>
#include "call_session.h"
#include <memory>
#include "logger.h"

class CallSessionManager
{
private:
    std::map<std::string, std::shared_ptr<CallSession>> m_sessions; // ✅ FIXED
    std::mutex m_mutex;
    Clogger* m_log;
    std::string logMsg;

public:
    CallSessionManager(Clogger* logger);
    CallSessionManager();
    void addSession(std::shared_ptr<CallSession> session);
    CallSession* getSession(const std::string& callID);
    void removeSession(const std::string& callID);
    bool hasSession(const std::string& callID);
    void printAllSessions();
};

#endif