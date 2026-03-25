#ifndef CALL_SESSION_H
#define CALL_SESSION_H

#include <string>
#include "rtp_session.h"   // 🔥 ADD THIS
#include "types.h"

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

    //  ADD THIS (VERY IMPORTANT)
    RTPSession rtp;

    // State
    std::string state;

    CallSession();

    bool isCaller(const std::string& ip, uword port) const;
};

#endif