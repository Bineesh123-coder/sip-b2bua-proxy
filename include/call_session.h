#ifndef CALL_SESSION_H
#define CALL_SESSION_H

#include <string>

typedef unsigned short uword;

class CallSession
{
public:
    std::string callID;

    // Caller (UAC)
    std::string callerIP;
    uword callerPort;

    // Callee (UAS)
    std::string targetIP;
    uword targetPort;

    // State (optional but useful)
    std::string state; // INVITE_SENT, RINGING, CONNECTED

    CallSession();

    bool isCaller(const std::string& ip, uword port) const;
};

#endif