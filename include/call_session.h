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
    std::string callerUser;
    std::string callertoTag;
    uword callerPort;

    // ===== CALLEE SIDE =====
    std::string calleeCallID;
    std::string calleeIP;
    uword calleePort;
    std::string calleeFrom;
    std::string calleeTo;
    std::string calleetoTag;
    std::string calleefromTag;
    std::string calleeBranch;
    std::string calleeUser;
    int calleeCSeq;

    std::string serverTagForCaller;
    std::string serverTagForCallee;
    
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

    bool isTerminated;

    int byeConfirmedCount;
    
    CallSession();

    bool isCaller(const std::string& ip, uword port) const;
};

#endif