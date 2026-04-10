#ifndef SIP_PARSER_H
#define SIP_PARSER_H

#include <string>
#include <unordered_map>

class SIPParser
{
public:
    SIPParser();

    std::string m_rawData;   // ✅ store full SIP message

    // Parse full SIP message
    void parse(const std::string& data);

    // Get SIP method (INVITE, ACK, etc.)
    std::string getMethod() const;

    // Get specific header
    std::string getHeader(const std::string& key) const;

    // Convenience getters
    std::string getCallID() const;
    std::string getFrom() const;
    std::string getTo() const;
    std::string getVia() const;
    std::string getCSeq() const;
    std::string getCSeqMethod() const;
    std::string getRequestLine() const;
    void parseRequestURI(const std::string& line,
                     std::string& ip,
                     int& port) const;
    std::string getFromUser() const;
    std::string getToUser() const;
    std::string getContact() const;
    std::string getViaBranch() const;
    

private:
    std::string m_method;
    std::unordered_map<std::string, std::string> m_headers;
    std::string trim(const std::string& str) const;
};

#endif