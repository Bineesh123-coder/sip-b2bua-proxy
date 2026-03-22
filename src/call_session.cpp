#include "call_session.h"

CallSession::CallSession()
{
    callerPort = 0;
    targetPort = 0;
    state = "INIT";
}

bool CallSession::isCaller(const std::string& ip, uword port) const {
    return ip == callerIP && port == callerPort;
}