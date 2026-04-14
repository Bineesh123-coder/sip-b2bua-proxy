#ifndef RTP_RELAY_H
#define RTP_RELAY_H

#include "rtp_session.h"
#include <memory>
//#include "logger.h"

void rtpRelayWorker(RTPSession &session);
void rtpRelayWorker(std::shared_ptr<RTPSession> session);

int allocateRTPPort();

#endif