#include "call_session.h"
#include <iostream>
#include <memory>

CallSession::CallSession()
{
    callerPort = 0;
    calleePort = 0;
    state = "INIT";
    rtp = std::make_shared<RTPSession>();
}

bool CallSession::isCaller(const std::string& ip, uword port) const {
    try{

         return ip == callerIP && port == callerPort;
    }
    catch (const std::exception &e)
    {
        std::cout << "ERROR: CallSession::isCaller:" << e.what() << std::endl;
        return false;

    }
   
}