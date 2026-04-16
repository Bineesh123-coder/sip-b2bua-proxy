#ifndef ENVREADER_H
#define ENVREADER_H

#include <string>
#include <map>
#include "logger.h"

class EnvReader {
public:
    EnvReader();
    ~EnvReader();
    // Constructor to initialize the .env file path
    EnvReader(Clogger* logger);

    void SetLog(Clogger* pLog);

    // Load and parse the .env file
    bool load();

    // Get the value associated with a specific key
    std::string getValue(const std::string& key) const;

    // Get all key-value pairs from the .env file
    std::map<std::string, std::string> getAll() const;

    // Set or update the value for a specific key
    void setValue(const std::string& key, const std::string& value);

    // Save the current key-value pairs back to the .env file
    bool save() const;

    void clearenvmap()
    {
        envMap.clear();
    }

private:
    // Helper function to trim whitespace from the beginning and end of a string
    std::string trim(const std::string& str) const;
    std::string m_filename; // Path to the .env file
    std::map<std::string, std::string> envMap; // Map to store key-value pairs
    std::string getExecutablePath();
    Clogger* m_pLog;
};

#endif // ENVREADER_H
