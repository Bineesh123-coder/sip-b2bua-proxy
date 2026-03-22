#include "call_session_manager.h"
#include <iostream>

CallSessionManager::CallSessionManager()
{
}

// Add new session
void CallSessionManager::addSession(const CallSession& session)
{
    std::lock_guard<std::mutex> lock(m_mutex);

    auto it = m_sessions.find(session.callID);
    if (it != m_sessions.end())
    {
        std::cout << "[WARN] Session already exists: " << session.callID << "\n";
        return;
    }

    m_sessions[session.callID] = session;

    std::cout << "[INFO] Session created: " << session.callID << "\n";
}

// Get session
CallSession* CallSessionManager::getSession(const std::string& callID)
{
    std::lock_guard<std::mutex> lock(m_mutex);

    auto it = m_sessions.find(callID);
    if (it != m_sessions.end())
    {
        return &(it->second);
    }

    return nullptr;
}

// Remove session
void CallSessionManager::removeSession(const std::string& callID)
{
    std::lock_guard<std::mutex> lock(m_mutex);

    auto it = m_sessions.find(callID);
    if (it != m_sessions.end())
    {
        m_sessions.erase(it);
        std::cout << "[INFO] Session removed: " << callID << "\n";
    }
}

// Check if session exists
bool CallSessionManager::hasSession(const std::string& callID)
{
    std::lock_guard<std::mutex> lock(m_mutex);
    return (m_sessions.find(callID) != m_sessions.end());
}

// Debug: print all sessions
void CallSessionManager::printAllSessions()
{
    std::lock_guard<std::mutex> lock(m_mutex);

    std::cout << "\n==== ACTIVE SESSIONS ====\n";

    for (const auto& pair : m_sessions)
    {
        const CallSession& s = pair.second;

        std::cout << "Call-ID: " << s.callID << "\n";
        std::cout << "  Caller: " << s.callerIP << ":" << s.callerPort << "\n";
        std::cout << "  Callee: " << s.targetIP << ":" << s.targetPort << "\n";
        std::cout << "  State : " << s.state << "\n\n";
    }

    std::cout << "=========================\n";
}