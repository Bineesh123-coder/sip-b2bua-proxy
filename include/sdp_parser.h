#ifndef SDP_PARSER_H
#define SDP_PARSER_H

#include <string>
#include <regex>

struct SDPInfo {
    std::string ip;
    int port = 0;
};

class SDPParser {
public:
    static SDPInfo parse(const std::string& sdp);
    static std::string extractSDP(const std::string& data);
    static std::string extractHeaders(const std::string& data);

    static std::string modifyAudioPort(const std::string& sdp, int newPort);
    static std::string modifyConnectionIP(const std::string& sdp, const std::string& ip);

    static std::string updateContentLength(const std::string& headers, int sdpLength);
    static std::string cleanSDP(const std::string& sdp);
    static std::string modifyContact(const std::string& headers, const std::string& serverIP);
    static std::string trim(const std::string& str);
    static std::string fixToHeader(const std::string& headers, const std::string& originalTo);
    static std::string modifyContact(const std::string& headers,
                          const std::string& user,
                          const std::string& ip,
                          int port);
    
};

#endif