#ifndef CALL_SESSION_H
#define CALL_SESSION_H

#include <string>
#include "rtp_session.h"  
#include "types.h"

class CallSession
{
public:

     // ===== CALLER SIDE =====
    std::string callerVia;
    std::string callerFrom;
    std::string callerTo;
    std::string callerCallID;
    std::string callerCSeq;
    std::string callerIP;
    std::string callersdp;
    uword callerPort;

    // ===== CALLEE SIDE =====
    std::string calleeCallID;
    std::string calleeIP;
    uword calleePort;
    std::string calleeFrom;
    std::string calleeTo;
    int calleeCSeq;

    // ===== B2BUA CONTROL =====
    std::string toTag;        // generated once
    std::string fromTag;      // optional if needed
    
    int CSeq;
    // RTP
    int rtp_port;


    //  ADD THIS (VERY IMPORTANT)
    RTPSession rtp;

    // State
    std::string state;

    CallSession();

    bool isCaller(const std::string& ip, uword port) const;
};

#endif