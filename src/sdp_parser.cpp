#include "sdp_parser.h"
#include <sstream>
#include <iostream>

SDPInfo SDPParser::parse(const std::string& sdp) {
    SDPInfo info;

     try{

        std::istringstream stream(sdp);
        std::string line;

        while (std::getline(stream, line)) {

            if (line.find("c=IN IP4") != std::string::npos) {
                info.ip = line.substr(line.find_last_of(" ") + 1);
            }

            if (line.find("m=audio") != std::string::npos) {
                std::istringstream l(line);
                std::string tmp;
                l >> tmp;
                l >> info.port;
            }
        }

        return info;
    }
    catch (const std::exception &e)
    {
        std::cout << "ERROR: SDPParser::extractSDP: " << e.what() << std::endl;
        return info;
    }

    
}

std::string SDPParser::extractSDP(const std::string& data) {
    try{

        size_t pos = data.find("\r\n\r\n");
        if (pos == std::string::npos) return "";
        return data.substr(pos + 4);
    }
    catch (const std::exception &e)
    {
        std::cout << "ERROR: SDPParser::extractSDP: " << e.what() << std::endl;
        return "";
    }
}

std::string SDPParser::extractHeaders(const std::string& data) {
    try {
        size_t pos = data.find("\r\n\r\n");
        if (pos == std::string::npos) return data;

        // ✅ DO NOT include last empty line
        return data.substr(0, pos + 2);  // only one \r\n
    }
    catch (const std::exception &e) {
        std::cout << "ERROR: SDPParser::extractHeaders: " << e.what() << std::endl;
        return "";
    }
}

std::string SDPParser::modifyAudioPort(const std::string& sdp, int newPort)
{
    try {
        std::istringstream stream(sdp);
        std::string line, result;

        while (std::getline(stream, line)) {

            if (line.find("m=audio") == 0) {

                // Find "RTP/AVP"
                size_t pos = line.find("RTP/AVP");

                if (pos != std::string::npos) {
                    // Keep codecs (everything after RTP/AVP)
                    std::string codecs = line.substr(pos + 7); // includes space

                    result += "m=audio " + std::to_string(newPort) + " RTP/AVP" + codecs + "\r\n";
                } else {
                    // fallback (rare)
                    result += "m=audio " + std::to_string(newPort) + " RTP/AVP 0 8 3 101\r\n";
                }
            }
            else {
                result += line + "\r\n";
            }
        }

        return result;
    }
    catch (const std::exception &e) {
        std::cout << "ERROR: modifyAudioPort: " << e.what() << std::endl;
        return "";
    }
}

std::string SDPParser::modifyConnectionIP(const std::string& sdp, const std::string& newIP)
{   
    try{
        std::istringstream stream(sdp);
        std::string line, result;

        while (std::getline(stream, line))
        {
            // ✅ ONLY modify connection line
            if (line.find("c=IN IP4") != std::string::npos)
            {
                line = "c=IN IP4 " + newIP;
            }

            // ❌ DO NOT TOUCH o= line

            result += line + "\r\n";
        }

        return result;
    }
    catch (const std::exception &e)
    {
        std::cout << "ERROR: SDPParser::modifyConnectionIP: " << e.what() << std::endl;
        return "";
    }
}

std::string SDPParser::updateContentLength(const std::string& headers, int sdpLength) {

    try{

        std::string modified = headers;

        size_t pos = modified.find("Content-Length:");
        if (pos != std::string::npos) {
            size_t end = modified.find("\r\n", pos);
            modified.replace(pos, end - pos,
                "Content-Length: " + std::to_string(sdpLength));
        }

        return modified;
    }
    catch (const std::exception &e)
    {
        std::cout << "ERROR: SDPParser::updateContentLength: " << e.what() << std::endl;
        return "";
    }
    
}

std::string SDPParser::cleanSDP(const std::string& sdp)
{
    try {
        std::istringstream stream(sdp);
        std::string line, result;

        while (std::getline(stream, line))
        {
            // 🔥 trim CR and spaces
            line.erase(std::remove(line.begin(), line.end(), '\r'), line.end());

            // skip truly empty lines
            if (line.find_first_not_of(" \t") == std::string::npos)
                continue;

            result += line + "\r\n";
        }

        return result;
    }
    catch (const std::exception &e)
    {
        std::cout << "ERROR: cleanSDP: " << e.what() << std::endl;
        return "";
    }
}

std::string SDPParser::modifyContact(const std::string& headers,
                                     const std::string& serverIP)
{
    try {
        std::istringstream stream(headers);
        std::string line, result;

        while (std::getline(stream, line)) {

            if (line.find("Contact:") != std::string::npos) {

                // extract username from original Contact
                size_t sipPos = line.find("sip:");
                size_t atPos = line.find("@");

                std::string user = "user"; // fallback

                if (sipPos != std::string::npos && atPos != std::string::npos) {
                    user = line.substr(sipPos + 4, atPos - (sipPos + 4));
                }

                // rebuild correct Contact
                line = "Contact: <sip:" + user + "@" + serverIP + ":5060>";
            }

            result += line + "\r\n";
        }

        return result;
    }
    catch (const std::exception &e) {
        std::cout << "ERROR: modifyContact: " << e.what() << std::endl;
        return "";
    }
}

std::string SDPParser::trim(const std::string& str)
{   
    try{

        size_t first = str.find_first_not_of(" \r\n\t");
        size_t last  = str.find_last_not_of(" \r\n\t");

        if (first == std::string::npos)
            return "";

        return str.substr(first, last - first + 1);
    }
    catch (const std::exception &e)
    {
        std::cout << "ERROR: SDPParser::trim: " << e.what() << std::endl;
        return "";
    }
    
}

std::string SDPParser::fixToHeader(const std::string& headers,
                                   const std::string& originalTo)
{
    std::istringstream stream(headers);
    std::string line, result;

    std::string tag;

    while (std::getline(stream, line))
    {
        if (line.find("To:") != std::string::npos)
        {
            // extract tag
            size_t tagPos = line.find("tag=");
            if (tagPos != std::string::npos)
            {
                tag = line.substr(tagPos);
            }

            // ALWAYS rebuild properly
            std::string newTo = "To: ";

            // remove "To: " if already exists in originalTo
            std::string cleanTo = originalTo;
            if (cleanTo.find("To:") != std::string::npos)
            {
                cleanTo = cleanTo.substr(3);
            }

            newTo += cleanTo;

            if (!tag.empty())
            {
                newTo += ";" + tag;
            }

            line = newTo;
        }

        result += line + "\r\n";
    }

    return result;
}

std::string SDPParser::modifyContact(const std::string& headers,
                          const std::string& user,
                          const std::string& ip,
                          int port)
{
    std::regex contactRegex("Contact:.*\r\n");

    std::string newContact =
        "Contact: <sip:" + user + "@" + ip + ":" +
        std::to_string(port) + ">\r\n";

    return std::regex_replace(headers, contactRegex, newContact);
}