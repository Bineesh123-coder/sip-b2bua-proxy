#include "call_session_manager.h"
#include <iostream>

CallSessionManager::CallSessionManager()
{   
   
}

CallSessionManager::CallSessionManager(Clogger* logger)
{   
    m_log =  logger;
}



// Add new session (UPDATED)
void CallSessionManager::addSession(std::shared_ptr<CallSession> session)
{   
    try{

        std::lock_guard<std::mutex> lock(m_mutex);

        auto it = m_sessions.find(session->calleeCallID);
        if (it != m_sessions.end())
        {
            logMsg ="[WARN] Session already exists: " + session->calleeCallID;
            std::cout<<logMsg<<std::endl;
            m_log->WriteLog(kDebug, logMsg);
             
            return;
        }

        m_sessions[session->calleeCallID] = session;
        m_sessions_by_caller[session->callerCallID] = session;
        m_sessions_by_callee[session->calleeCallID] = session;


        logMsg ="[INFO] Session created: " + session->calleeCallID;
        std::cout<<logMsg<<std::endl;
        m_log->WriteLog(kDebug, logMsg);
    }
    catch (const std::exception &e)
    {
        std::cout << "ERROR: addSession: " << e.what() << std::endl;
    }
    
}

// Get session
CallSession* CallSessionManager::getSession(const std::string& callID)
{   
    try{

        std::lock_guard<std::mutex> lock(m_mutex);

        auto it = m_sessions.find(callID);
        if (it != m_sessions.end())
        {
            return it->second.get();  //  FIX (pointer from shared_ptr)
        }

        return nullptr;
    }
    catch (const std::exception &e)
    {
        std::cout << "ERROR: getSession: " << e.what() << std::endl;
        return nullptr;
    }
    
}

// Remove session
void CallSessionManager::removeSession(const std::string& callID)
{   
    try{

        std::lock_guard<std::mutex> lock(m_mutex);

        auto it = m_sessions.find(callID);
        if (it != m_sessions.end())
        {
            m_sessions.erase(it);
            logMsg ="[INFO] Session removed: " + callID;
            std::cout<<logMsg<<std::endl;
            m_log->WriteLog(kDebug, logMsg);
        }
    }
    catch (const std::exception &e)
    {
        std::cout << "ERROR: removeSession: " << e.what() << std::endl;
    }
    
}

// Check if session exists
bool CallSessionManager::hasSession(const std::string& callID)
{
    try
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        return (m_sessions.find(callID) != m_sessions.end());
    }
    catch (const std::exception& e)
    {
        std::cerr << "Exception in hasSession: " << e.what() << std::endl;
        return false;
    }
}
// Debug: print all sessions
void CallSessionManager::printAllSessions()
{   
    try{

        std::lock_guard<std::mutex> lock(m_mutex);

        std::cout << "\n==== ACTIVE SESSIONS ====\n";

        for (const auto& pair : m_sessions)
        {
            const std::shared_ptr<CallSession>& s = pair.second;

            std::cout << "Call-ID: " << s->calleeCallID << "\n";
            std::cout << "  Caller: " << s->callerIP << ":" << s->callerPort << "\n";
            std::cout << "  Callee: " << s->calleeIP << ":" << s->calleePort << "\n";
            std::cout << "  State : " << s->state << "\n\n";
            logMsg ="Call-ID:  " + s->calleeCallID +" Caller:"+ s->callerIP+ "Callee: "+s->calleeIP+"State: "+s->state;
            std::cout<<logMsg<<std::endl;
            m_log->WriteLog(kDebug, logMsg); 
        }

        std::cout << "=========================\n";
    }
    catch (const std::exception &e)
    {
        std::cout << "ERROR: printAllSessions: " << e.what() << std::endl;
    }
    
}

// CallSession* CallSessionManager::getSessionByCalleeCallID(const std::string& callID)
// {
//     try
//     {
//         std::lock_guard<std::mutex> lock(m_mutex);

//         for (auto& it : m_sessions)
//         {
//             if (it.second->calleeCallID == callID)
//             {
//                 return it.second.get();
//             }
//         }

//         return nullptr;
//     }
//     catch (const std::exception &e)
//     {
//         std::cout << "ERROR: getSessionByCalleeCallID: " << e.what() << std::endl;
//         return nullptr;
//     }
// }

std::shared_ptr<CallSession>
CallSessionManager::getSessionByCallerCallID(const std::string& callID)
{
    std::lock_guard<std::mutex> lock(m_mutex);

    auto it = m_sessions_by_caller.find(callID);
    if (it != m_sessions_by_caller.end())
    {
        return it->second;   // ✅ return shared_ptr
    }

    return nullptr;
}

std::shared_ptr<CallSession>
CallSessionManager::getSessionByCalleeCallID(const std::string& callID)
{
    std::lock_guard<std::mutex> lock(m_mutex);

    auto it = m_sessions_by_callee.find(callID);
    if (it != m_sessions_by_callee.end())
    {
        return it->second;   // ✅ return shared_ptr
    }

    return nullptr;
}