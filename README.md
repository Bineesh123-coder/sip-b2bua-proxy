🚀 SIP B2BUA Proxy Engine

High-Performance C++ SIP Proxy with RTP Relay & Real-Time Monitoring

📌 Overview

This project is a SIP Back-to-Back User Agent (B2BUA) built in C++, designed to handle real-time VoIP call signaling and RTP media relay with high performance and low latency.

It simulates a production-grade SIP server by managing:

SIP signaling (INVITE, REGISTER, BYE, etc.)
RTP packet forwarding (bidirectional media relay)
Call session lifecycle
Real-time monitoring using Redis

⚙️ Features

📞 SIP Signaling
Handles core SIP methods:
REGISTER
INVITE
ACK
BYE
B2BUA architecture (acts as both client & server)
Session-based call handling

🎧 RTP Media Relay

Bidirectional RTP forwarding (Caller ↔ Callee)
Dynamic RTP port allocation
Real-time packet routing
Lightweight and optimized loop for low latency

📊 Real-Time Monitoring (Redis)

Live call tracking
Device state management
Call Detail Records (CDR)
Redis Data Model:
ACTIVE_CALLS → Set of active call IDs
CALL:<id> → Live call metrics
DEVICE:<user> → Device state (IDLE, BUSY)
CDR:<id> → Final call summary
USER:<user>:CALLS → Call history

📈 Call Metrics

Packet count (in/out)
Packet loss
Average jitter
Call duration

🧠 Session Management

Tracks full call lifecycle
Handles duplicate INVITE protection
Maintains RTP session state
Safe call termination handling

🏗️ Architecture

Caller (SIPp/Phone)
        │
        ▼
   ┌──────────────┐
   │   B2BUA      │
   │ (C++ Server) │
   └──────┬───────┘
          │
          ▼
Callee (SIPp/Phone)

        │
        ▼
   RTP Relay Engine
 (Bidirectional Media)

        │
        ▼
      Redis
 (Live + History Data)

🛠️ Technologies Used

C++ (Core Implementation)
UDP Sockets (SIP + RTP)
Redis (Monitoring & Data Storage)
SIP Protocol (RFC 3261)
RTP Protocol

▶️ How to Run

1️⃣ Start Redis
redis-server

2️⃣ Build Project
make clean
make

3️⃣ Run Server
./sip-server

🧪 Testing

🔹 Manual Testing (Softphones)
Register two users (e.g., 1001, 1005)
Make call → verify audio + logs
🔹 Load Testing (Recommended)

Use SIP traffic generator:

sipp <server-ip>:5060 -sn uac -s 1001 -r 5 -m 50 -rtp_echo
Parameters:
-r → Calls per second
-m → Total calls
-rtp_echo → RTP testing
🔍 Redis Debug Commands
KEYS *
HGETALL DEVICE:1001
SMEMBERS ACTIVE_CALLS
SMEMBERS USER:1001:CALLS
HGETALL CDR:<callId>

📊 Example Call Summary
===== CALL SUMMARY =====
Call-ID      : abc123
Caller       : 1005
Callee       : 1001
Duration     : 45 seconds
Total Packets: 4575
Packet Loss  : 3
Avg Jitter   : 3.14 ms

⚡ Performance Highlights

Handles concurrent SIP sessions efficiently
Optimized RTP loop (minimal latency)
Supports load testing with multiple simultaneous calls
Real-time monitoring without blocking RTP thread

⚠️ Known Limitations

No NAT traversal (STUN/TURN not implemented)
Basic SDP handling
No TLS/SRTP support
Simple Redis schema (can be extended)
🚀 Future Improvements
Add Web Dashboard (live call monitoring)
RTP jitter buffer implementation
NAT traversal support
Async Redis (pipeline / pub-sub)
Call recording support
🎯 Project Goal

To simulate a real-world telecom backend system with:

Real-time signaling
Media handling
Monitoring & analytics
👨‍💻 Author

Bineesh M B

💬 Final Note

This project demonstrates:

Strong understanding of SIP & RTP
Real-time system design
Backend performance optimization