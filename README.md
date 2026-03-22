# sip-b2bua-proxy
SIP B2BUA Proxy Engine – High-Performance C++ with RTP Media Relay & Real-Time Monitoring

Built a high-performance SIP B2BUA proxy in C++ handling real-time call signaling and RTP media relay. Implemented session management, bidirectional RTP forwarding, and Redis-based monitoring for live call metrics. Load-tested system using SIPp with concurrent call simulation and optimized for low-latency packet processing.

Insatll sipp 

✅ Step 1: Clone stable version (NOT master)
git clone https://github.com/SIPp/sipp.git
cd sipp
git checkout v3.7.3

👉 This avoids the broken test dependency issue

✅ Step 2: Build normally
cmake .
make -j$(nproc)
✅ Step 3: Install
sudo make install
✅ Step 4: Verify
sipp -v


sip-b2bua-proxy-engine/
│
├── src/
│   ├── main.cpp
│   │
│   ├── sip/
│   │   ├── sip_server.cpp
│   │   ├── sip_server.h
│   │   ├── sip_parser.cpp
│   │   ├── sip_parser.h
│   │   ├── sip_message.h
│   │
│   ├── rtp/
│   │   ├── rtp_relay.cpp
│   │   ├── rtp_relay.h
│   │
│   ├── session/
│   │   ├── call_session.cpp
│   │   ├── call_session.h
│   │   ├── session_manager.cpp
│   │   ├── session_manager.h
│   │ 
│   ├── network/
│   │   ├── udp_socket.cpp
│   │   ├── udp_socket.h
│   │
│   ├── redis/
│   │   ├── redis_client.cpp
│   │   ├── redis_client.h
│   │
│   ├── utils/
│   │   ├── logger.cpp
│   │   ├── logger.h
│
├── include/   (optional, if you separate headers)
│
├── config/
│   ├── config.json
│
├── tests/
│
├── Makefile
├── README.md


test call
sipp -sn uac 127.0.0.1:5060


✅ 2. REAL TEST (RECOMMENDED)

👉 Use 2 instances of SIPp

🧪 Step 1: Start UAS (Receiver)
sipp -sn uas -i 127.0.0.1 -p 5061
🧪 Step 2: Start Your Server
./sip_proxy
🧪 Step 3: Start UAC (Caller)
sipp -sn uac 127.0.0.1:5060
🔥 Now Flow is:
UAC → YOU → UAS
INVITE →

UAS → YOU → UAC
200 OK ←

UAC → YOU → UAS
ACK →

✔ This is real SIP behavior


# terminal 1
sipp -sn uas -p 5061

# terminal 2
./sip_proxy

# terminal 3
sipp -sn uac 127.0.0.1:5060 -trace_msg


✅ FULL CORRECT TEST (CLEAN)
🧪 Terminal 1 (UAS)

sipp -sn uas -p 5061

🧪 Terminal 2 (Your Server)

./sip_proxy

🧪 Terminal 3 (UAC → ONLY ONE CALL)

sipp -sn uac 127.0.0.1:5060 -m 1 -trace_msg
