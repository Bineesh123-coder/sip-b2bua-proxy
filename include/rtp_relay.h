#ifndef RTP_RELAY_H
#define RTP_RELAY_H

#include "rtp_session.h"


void rtpRelayWorker(RTPSession &session);

int allocateRTPPort();

#endif