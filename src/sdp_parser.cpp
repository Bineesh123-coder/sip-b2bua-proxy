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

    try{

        size_t pos = data.find("\r\n\r\n");
        if (pos == std::string::npos) return data;
        return data.substr(0, pos + 4);
    }
    catch (const std::exception &e)
    {
        std::cout << "ERROR: SDPParser::extractHeaders: " << e.what() << std::endl;
        return "";
    }
    
}

std::string SDPParser::modifyAudioPort(const std::string& sdp, int newPort) {

    try{

        std::istringstream stream(sdp);
        std::string line, result;

        while (std::getline(stream, line)) {
            if (line.find("m=audio") != std::string::npos) {
                result += "m=audio " + std::to_string(newPort) + " RTP/AVP 0\r\n";
            } else {
                result += line + "\r\n";
            }
        }

        return result;
    }
    catch (const std::exception &e)
    {
        std::cout << "ERROR: SDPParser::modifyAudioPort: " << e.what() << std::endl;
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
            if (line.find("c=IN IP4") != std::string::npos)
            {
                line = "c=IN IP4 " + newIP;
            }
            else if (line.find("o=") == 0)
            {
                // replace last IP in o=
                size_t pos = line.find_last_of(" ");
                if (pos != std::string::npos)
                    line = line.substr(0, pos + 1) + newIP;
            }

            result += line + "\r\n";
        }

        return result;
    }
    catch (const std::exception &e)
    {
        std::cout << "ERROR: SDPParser::modifyAudioPort: " << e.what() << std::endl;
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
    try{

        std::istringstream stream(sdp);
        std::string line, result;

        while (std::getline(stream, line))
        {
            if (line.empty()) continue;  //  remove blank lines
            result += line + "\r\n";
        }

        return result;
    }
    catch (const std::exception &e)
    {
        std::cout << "ERROR: SDPParser::cleanSDP: " << e.what() << std::endl;
        return "";
    }
    
}
