#ifndef CLOGGER_H
#define CLOGGER_H

#include "constants.h"
#include <string>
#include <cstdio>
#include <pthread.h>
#include <chrono>
#include <cstring> // For std::strlen
#include <ctime>
#include <sys/stat.h> // For mkdir

/**
 * Clogger class for logging functionality
 */

class Clogger {
public:
    Clogger(const std::string& dataPath, int debugLevel, int loggingEnabled, const std::string& sFolderName);
    Clogger(const std::string& dataPath, int debugLevel, int loggingEnabled, const std::string& sFolderName, const std::string& channelId); // Constructor
    ~Clogger(); // Destructor
    // Creates a log file for each hour
    void CreateLog();
    void CreateLog(const std::string& channelId);
    // Enable or disable logging
    void SetLoggingEnabled(int enabled);
    // Set the data path for log files
    void SetDataPath(const std::string& dataPath);
    // Set the debug level
    void SetDebugLevel(int debugLevel);
    // Close the log file if open
    void closeLog();
    // Write a log entry with the specified level
    int WriteLog(int level, const std::string& message);
    FILE* m_fp; // File pointer for the log file

private:
    // Creates a log file with the specified parameters
    int CreateHourLog(const std::string& filename, const std::string& dataPath, const std::string& dirName);
    int m_debugLevel;       // Debug level
    std::string m_sDataPath; // Data path for log storage
    std::string m_sFolderName;
    int m_nYear;            // Current year
    int m_nMonth;           // Current month
    int m_nDay;             // Current day
    int m_nHour;            // Current hour
    char m_logString[500];  // Buffer for log messages
    int m_loggingEnabled;  // Flag to control logging
    pthread_mutex_t m_lock; // Pointer to mutex for thread-safe operations
    std::string m_channelId;
    
    
};

#endif // CLOGGER_H