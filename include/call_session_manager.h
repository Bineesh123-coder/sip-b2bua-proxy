#ifndef CALL_SESSION_MANAGER_H
#define CALL_SESSION_MANAGER_H

#include <map>
#include <string>
#include <mutex>
#include "call_session.h"

class CallSessionManager
{
private:
    std::map<std::string, CallSession> m_sessions;
    std::mutex m_mutex;  // thread safety

public:
    CallSessionManager();

    // Create new session
    void addSession(const CallSession& session);

    // Get session pointer (NULL if not found)
    CallSession* getSession(const std::string& callID);

    // Remove session
    void removeSession(const std::string& callID);

    // Check existence
    bool hasSession(const std::string& callID);

    // Debug (optional)
    void printAllSessions();
};

#endif