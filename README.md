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

for cancel 

sipp -sf uas_delay.xml -p 5061

./sip_proxy

sipp -sn uac 127.0.0.1:5060 -m 1 -timeout 2000 -trace_msg


# Terminal 1
sipp -sf uas_delay.xml -p 5061

# Terminal 2
./sip_proxy

# Terminal 3
sipp -sf uac_cancel.xml 127.0.0.1:5060 -p 5062 -m 1 -trace_msg


🔥 STEP 2 — RUN SIPp WITH RTP
Terminal 1 (UAS)
sipp -sn uas -p 5061 -rtp_echo
Terminal 2 (Proxy)
./sip_proxy
Terminal 3 (UAC)
sipp -sn uac 127.0.0.1:5060 -m 1 -rtp_echo -trace_msg
sipp -sn uac 192.168.2.255:5060 -m 1 -rtp_echo -trace_msg
sipp -sn uac 192.168.1.7:5060 -m 1 -rtp_echo -trace_msg


chmod -R 777 /opt/app/DATA/LOG/Sip_Server


Caller              B2BUA               Callee
  | INVITE --------> |                   |
  |                 | INVITE ---------> |
  |                 | <------ 100 ------|
  | <------ 100 ----|                   |
  |                 | <------ 180 ------|
  | <------ 180 ----|                   |
  |                 | <------ 200 ------|
  | <------ 200 ----|                   |
  | ACK ----------> |                   |
  |                 | ACK ----------->  |
  | RTP <=========> | <=============>   |
  | BYE ----------> |                   |
  |                 | BYE ----------->  |
  |                 | <------ 200 ------|
  | <------ 200 ----|                   |



1001 REGISTER → 200 OK
1004 REGISTER → 200 OK

1004 → INVITE → Proxy
Proxy → 100 Trying → 1004

Proxy → INVITE → 1001

1001 → 100 Trying → Proxy → 1004
1001 → 180 Ringing → Proxy → 1004
1001 → 200 OK → Proxy → 1004

1004 → ACK → Proxy → 1001

 CALL ESTABLISHED