#include "sip_parser.h"
#include <sstream>
#include <algorithm>

SIPParser::SIPParser()
{
}

void SIPParser::parse(const std::string& data)
{
    m_headers.clear();
    m_method.clear();

    m_rawData = data;

    std::istringstream stream(data);
    std::string line;

    // 👉 First line = method
    if (std::getline(stream, line))
    {
        std::istringstream firstLine(line);
        firstLine >> m_method;
    }

    // 👉 Parse headers
    while (std::getline(stream, line))
    {
        if (line == "\r" || line.empty())
            break;

        size_t pos = line.find(":");
        if (pos != std::string::npos)
        {
            std::string key = line.substr(0, pos);
            std::string value = line.substr(pos + 1);

            // trim spaces
            value.erase(0, value.find_first_not_of(" \t"));
            value.erase(value.find_last_not_of("\r\n") + 1);

            m_headers[key] = value;
        }
    }
}

std::string SIPParser::getMethod() const
{
    return m_method;
}

std::string SIPParser::getHeader(const std::string& headerName) const
{
    std::istringstream stream(m_rawData);
    std::string line;

    while (std::getline(stream, line))
    {
        // remove trailing \r
        if (!line.empty() && line.back() == '\r')
            line.pop_back();

        if (line.find(headerName + ":") == 0)
        {
            std::string value = line.substr(headerName.length() + 1);

            if (!value.empty() && value[0] == ' ')
                value.erase(0, 1);

            return trim(value);
        }
    }
    return "";
}

std::string SIPParser::getCallID() const
{
    return getHeader("Call-ID");
}

std::string SIPParser::getFrom() const
{
    return getHeader("From");
}

std::string SIPParser::getTo() const
{
    return getHeader("To");
}

std::string SIPParser::getVia() const
{
    return getHeader("Via");
}

std::string SIPParser::getCSeq() const
{
    return getHeader("CSeq");
}

std::string SIPParser::getCSeqMethod() const
{
    std::string cseq = getHeader("CSeq");

    std::istringstream iss(cseq);
    std::string number, method;

    iss >> number >> method;

    return method;
}

std::string SIPParser::getRequestLine() const
{
    std::istringstream stream(m_rawData);
    std::string line;

    if (std::getline(stream, line))
        return trim(line);

    return "";
}

void SIPParser::parseRequestURI(const std::string& line,
                     std::string& ip,
                     int& port) const
{
    size_t sipPos = line.find("sip:");
    size_t atPos = line.find("@");

    if (sipPos == std::string::npos || atPos == std::string::npos)
        return;

    size_t colonPos = line.find(":", atPos);
    size_t spacePos = line.find(" ", atPos);

    if (colonPos != std::string::npos && colonPos < spacePos)
    {
        ip = line.substr(atPos + 1, colonPos - (atPos + 1));
        port = std::stoi(line.substr(colonPos + 1,
                    spacePos - (colonPos + 1)));
    }
    else
    {
        ip = line.substr(atPos + 1, spacePos - (atPos + 1));
        port = 5060; // default
    }
}

std::string SIPParser::trim(const std::string& str) const
{
    size_t first = str.find_first_not_of(" \r\n\t");
    size_t last  = str.find_last_not_of(" \r\n\t");

    if (first == std::string::npos)
        return "";

    return str.substr(first, last - first + 1);
}