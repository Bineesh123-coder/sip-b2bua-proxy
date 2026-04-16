#ifndef CALL_SESSION_MANAGER_H
#define CALL_SESSION_MANAGER_H

#include <map>
#include <string>
#include <mutex>
#include "call_session.h"
#include <memory>
#include "logger.h"
#include <unordered_map>

class CallSessionManager
{
private:
    std::map<std::string, std::shared_ptr<CallSession>> m_sessions; // ✅ FIXED
    std::mutex m_mutex;
    Clogger* m_log;
    std::string logMsg;
    std::unordered_map<std::string, std::shared_ptr<CallSession>> m_sessions_by_caller;
    std::unordered_map<std::string, std::shared_ptr<CallSession>> m_sessions_by_callee;

public:
    CallSessionManager(Clogger* logger);
    CallSessionManager();
    void addSession(std::shared_ptr<CallSession> session);
    CallSession* getSession(const std::string& callID);
    void removeSession(const std::string& callID);
    bool hasSession(const std::string& callID);
    void printAllSessions();
    //CallSession* getSessionByCalleeCallID(const std::string& callID);
    //CallSession* getSessionByCallerCallID(const std::string& callID);
    std::shared_ptr<CallSession>getSessionByCallerCallID(const std::string& callID);
    std::shared_ptr<CallSession>getSessionByCalleeCallID(const std::string& callID);
};

#endif