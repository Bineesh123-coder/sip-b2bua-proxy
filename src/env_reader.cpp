#include "env_reader.h"
#include <fstream>
#include <sstream>
#include <algorithm>
#include <unistd.h>
#include <limits.h>
#include <iostream>
#include <stdexcept>

EnvReader::EnvReader()
{
    m_filename = getExecutablePath();
    //m_filename = "/opt/sip-b2bua-proxy";
    m_filename = m_filename + "/.env";
    m_pLog = nullptr;
    clearenvmap();

}

// Constructor to initialize the .env file path and LogWriter
EnvReader::EnvReader(Clogger* pLog)
    : m_pLog(pLog) {
    m_filename = ".env";
    clearenvmap();
}

// Destructor EnvReader
EnvReader::~EnvReader()
{
    clearenvmap();
}

void EnvReader::SetLog(Clogger* pLog)
{
    m_pLog = pLog;
}

// Function to get the executable path
std::string EnvReader::getExecutablePath() {
    try
    {
        char path[PATH_MAX] = { 0 };
        if (getcwd(path, sizeof(path)) == nullptr) {
            perror("getcwd");
            printf(("EnvReader: getExecutablePath ERROR: DEFAULT PATH: " + std::string(path) + "\n").c_str());
            return "/opt/sip-b2bua-proxy";
        }
        printf(("EnvReader: getExecutablePath: PATH: " + std::string(path) + "\n").c_str());
        return std::string(path);
    }
    catch (const std::exception& e) {
        printf(("getExecutablePath: ERROR: " + std::string(e.what()) + "\n").c_str());
        return "/opt/sip-b2bua-proxy"; // Return the original string on error
    }
}

// Helper function to trim whitespace from the beginning and end of a string
std::string EnvReader::trim(const std::string& str) const {
    try {
        size_t first = str.find_first_not_of(" \t\r\n");
        if (first == std::string::npos) return ""; // No non-space characters
        size_t last = str.find_last_not_of(" \t\r\n");
        return str.substr(first, (last - first + 1));
    }
    catch (const std::exception& e) {
        if (m_pLog)
        {
            m_pLog->WriteLog(kGeneralError, "Error trimming string: " + std::string(e.what()));
        }
        return str; // Return the original string on error
    }
}

// Load and parse the .env file
bool EnvReader::load() {
    try {
        std::ifstream file(m_filename);
        if (!file.is_open()) {
            if (m_pLog)
            {
                m_pLog->WriteLog(kGeneralError, "Error: Could not open the file " + m_filename);
            }
            std::string er = "Error: Could not open the file " + m_filename + ".\n";
            printf(er.c_str());
            return false;
        }

        std::string line;
        while (std::getline(file, line)) {
            // Skip empty lines or lines that start with a comment '#'
            if (line.empty() || line[0] == '#') {
                continue;
            }

            // Find the position of the first '=' character
            size_t pos = line.find('=');
            if (pos == std::string::npos) {
                if (m_pLog)
                {
                    m_pLog->WriteLog(kWarning, "Skipping malformed line: " + line);
                }

                continue;
            }

            // Split the line into key and value
            std::string key = trim(line.substr(0, pos));
            std::string value = trim(line.substr(pos + 1));

            // Insert the key-value pair into the map
            envMap[key] = value;
        }

        file.close();
        if (m_pLog)
        {
            m_pLog->WriteLog(kInformation, "Successfully loaded .env file");
        }
        printf("Successfully loaded.env file\n");
        return true;
    }
    catch (const std::exception& e) {
        if (m_pLog)
        {
            m_pLog->WriteLog(kGeneralError, "Error loading .env file: " + std::string(e.what()));
        }
        printf(("Error loading .env file: " + std::string(e.what()) + "\n").c_str());
        return false;
    }
}

// Get the value associated with a specific key
std::string EnvReader::getValue(const std::string& key) const {
    try {
        auto it = envMap.find(key);
        if (it != envMap.end()) {
            return it->second;
        }
        return ""; // Return empty string if key not found
    }
    catch (const std::exception& e) {
        if (m_pLog)
        {
            m_pLog->WriteLog(kGeneralError, "Error retrieving value for key " + key + ": " + e.what());
        }
        return ""; // Return empty string on error
    }
}

// Get all key-value pairs from the .env file
std::map<std::string, std::string> EnvReader::getAll() const {
    try {
        return envMap;
    }
    catch (const std::exception& e) {
        if (m_pLog)
        {
            m_pLog->WriteLog(kGeneralError, "Error retrieving all key-value pairs: " + std::string(e.what()));
        }
        return {}; // Return empty map on error
    }
}

// Set or update the value for a specific key
void EnvReader::setValue(const std::string& key, const std::string& value) {
    try {
        envMap[key] = value;
        if (m_pLog)
        {
            m_pLog->WriteLog(kInformation, "Set value for key " + key + " to " + value);
        }
    }
    catch (const std::exception& e) {
        if (m_pLog)
        {
            m_pLog->WriteLog(kGeneralError, "Error setting value for key " + key + ": " + e.what());
        }
    }
}

// Save the current key-value pairs back to the .env file
bool EnvReader::save() const {
    try {

        if (!envMap.empty())
        {
            std::ofstream file(m_filename);
            if (!file.is_open()) {
                if (m_pLog)
                {
                    m_pLog->WriteLog(kGeneralError, "Error: Could not open the file " + m_filename + " for writing");
                }
                return false;
            }

            for (const auto& pair : envMap) {
            const std::string& key = pair.first;   // Access the key
            const std::string& value = pair.second; // Access the value
            file << key << "=" << value << std::endl;
            }
            file.flush();
            file.close();
            if (m_pLog)
            {
                m_pLog->WriteLog(kInformation, "Successfully saved .env file");
            }
        }
        else
        {
            printf("ENV MAP EMPTY\n\n");
        }
        return true;
    }
    catch (const std::exception& e) {
        if (m_pLog)
        {
            m_pLog->WriteLog(kGeneralError, "Error saving .env file: " + std::string(e.what()));
        }
        return false;
    }
}
