#include "logger.h"
#include <iostream>
#include <fstream>
#include <iomanip> // for std::setw and std::setfill
#include <sstream> // for std::ostringstream

/**
 * Clogger constructor
 * @param dataPath - path where log files will be stored
 * @param debugLevel - debug level for logging
 * @param loggingEnabled - flag to control logging
 */
Clogger::Clogger(const std::string& dataPath, int debugLevel, int loggingEnabled, const std::string& sFolderName)
{
    try
    {
        m_sDataPath = dataPath;
        m_debugLevel = debugLevel;
        m_loggingEnabled = loggingEnabled;
        m_sFolderName = sFolderName;
        m_nYear = -1;
        m_nMonth = -1;
        m_nDay = -1;
        m_nHour = -1;
        m_fp = NULL;
        pthread_mutex_init(&m_lock, nullptr);
    }
    catch (...)
    {

    }
}

Clogger::Clogger(const std::string& dataPath, int debugLevel, int loggingEnabled, const std::string& sFolderName, const std::string& channelId)
{
    try
    {
        m_sDataPath = dataPath;
        m_debugLevel = debugLevel;
        m_loggingEnabled = loggingEnabled;
        m_sFolderName = sFolderName;
        m_channelId = channelId;
        m_nYear = -1;
        m_nMonth = -1;
        m_nDay = -1;
        m_nHour = -1;
        m_fp = NULL;
        pthread_mutex_init(&m_lock, nullptr);
    }
    catch (...)
    {

    }
}

/**
 * Clogger destructor
 */
Clogger::~Clogger() {
    try {
            closeLog(); // Close the log file on destruction
            pthread_mutex_destroy(&m_lock); // Destroy the mutex
    }
    catch (const std::exception& e) {
        std::cerr << "Exception in Clogger::~Clogger(): " << e.what() << std::endl;
    }
    catch (...) {
        std::cerr << "Unknown exception in Clogger::~Clogger()" << std::endl;
    }
}

/**
 * Close the log file if open
 */
 
 void Clogger::closeLog() {
     try {
         if (m_fp != nullptr) {
             fclose(m_fp); // Close the file
             m_fp = nullptr;
         }
     }
     catch (const std::exception& e) {
         std::cerr << "Exception in LogWriter::closeLog(): " << e.what() << std::endl;
     }
     catch (...) {
         std::cerr << "Unknown exception in LogWriter::closeLog()" << std::endl;
     }
 }

/**
 * Create a log file for each hour
 */
void Clogger::CreateLog() {
    //pthread_mutex_lock(&m_lock);
    try {
        
        if (m_loggingEnabled == 0) {
            return; // Skip log creation if logging is disabled
        }
        
        // Get the current time and convert it to local time
        auto now = std::chrono::system_clock::now();
        std::time_t now_c = std::chrono::system_clock::to_time_t(now);
        std::tm local_tm = *std::localtime(&now_c);

        // Check if a new log file is needed
        bool needNewLog = (m_nYear != local_tm.tm_year + 1900) || (m_nMonth != local_tm.tm_mon + 1) ||
            (m_nDay != local_tm.tm_mday) || (m_nHour != local_tm.tm_hour);

        // Construct the expected log filename
        /*
        std::string fileName = std::to_string(local_tm.tm_hour) + "_" + m_sFolderName + ".log";
        std::string logDir = m_sDataPath + "/LOG/" + m_sFolderName + "/" +
            std::to_string(m_nYear) + "-" +
            std::to_string(m_nMonth) + "-" +
            std::to_string(m_nDay);
        std::string fullFilePath = logDir + "/" + fileName;
        */


        /*
        bool fileExists = false;    
        std::ifstream file(fullFilePath);
        fileExists = file.good();
        */

        // Format hour for file name
        std::ostringstream hourStream;
        hourStream << std::setfill('0') << std::setw(2) << local_tm.tm_hour;

        // Format year, month, and day for the log directory
        std::ostringstream dateStream;
        dateStream << std::setfill('0')
            << std::setw(4) << m_nYear << "-"
            << std::setw(2) << m_nMonth << "-"
            << std::setw(2) << m_nDay;

        // Construct file name
        std::string fileName = hourStream.str() + "_" + m_sFolderName + ".log";

        // Construct log directory
        std::string logDir = m_sDataPath + "/LOG/" + m_sFolderName + "/" + dateStream.str();

        // Combine to get the full file path
        std::string fullFilePath = logDir + "/" + fileName;

        // Update the date and hour variables
        m_nYear = local_tm.tm_year + 1900;
        m_nMonth = local_tm.tm_mon + 1;
        m_nDay = local_tm.tm_mday;
        m_nHour = local_tm.tm_hour;

        //if (needNewLog || !fileExists) {
        if (needNewLog) {
            printf("\nON LOG CREATION\n");
            pthread_mutex_lock(&m_lock);
            try
            {
                this->closeLog(); // Close the existing log file
                CreateHourLog(fileName,  m_sDataPath, m_sFolderName);
            }
            catch (...)
            {
                std::cerr << "Exception in Clogger::CreateLog():CreateHourLog " << std::endl;
            }
            pthread_mutex_unlock(&m_lock);

            snprintf(m_logString, sizeof(m_logString), "[DCHL][RUN] Opened Sip Server  Log:");
            this->WriteLog(kGeneralError, m_logString);

        }  
    }
    catch (const std::exception& e) {
        std::cerr << "Exception in Clogger::CreateLog(): " << e.what() << std::endl;
    }
    catch (...) {
        std::cerr << "Unknown exception in Clogger::CreateLog()" << std::endl;
    }
    //pthread_mutex_unlock(&m_lock);
}


/**
 * Write a log entry with the specified level
 * @param level - debug level of the message
 * @param message - log message to be written
 * @return kSuccess on success, kFailure on failure
 */
 
 int Clogger::WriteLog(int level, const std::string& message) {
     int ret = kSuccess; // Return value to indicate success or failure
     pthread_mutex_lock(&m_lock);
     try {
         /*
         // Check if logging is enabled
         if (m_loggingEnabled == 0 || !m_fp) {
             ret = kFailure;
         }
         */
         if ((level <= m_debugLevel) && (m_fp)) {
            // Get the current time for the log entry
            auto now = std::chrono::system_clock::now();
            auto milliseconds = std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch()) % 1000;
            std::time_t now_c = std::chrono::system_clock::to_time_t(now);
            std::tm local_tm = *std::localtime(&now_c);

            // Format time as "HH:MM:SS.mmm"
            char tbuffer[100] = { 0 };
            std::strftime(tbuffer, sizeof(tbuffer), "%H:%M:%S", &local_tm);

            char tmp_str[2000] = { 0 };
            snprintf(tmp_str, sizeof(tmp_str), "Time:%s_%03d,Message:%s\n", tbuffer, static_cast<int>(milliseconds.count()), message.c_str());

            // Write and flush the log entry
            fwrite(tmp_str, std::strlen(tmp_str), 1, m_fp);
            fflush(m_fp);
        }
        else {
            ret = kFailure;
        }
     }
     catch (const std::exception& e) {
         std::cerr << "Exception in LogWriter::WriteLog(): " << e.what() << std::endl;
         ret = kFailure;
     }
     catch (...) {
         std::cerr << "Unknown exception in LogWriter::WriteLog()" << std::endl;
         ret = kFailure;
     }
     pthread_mutex_unlock(&m_lock);
     return ret;
 }

/**
 * Function to create a log file for each hour
 * @param filename - name of the log file to be created
 * @param t_debugLevel - debug level threshold
 * @param dataPath - base path for storing logs
 * @param dirName - directory name for the logs
 * @return kSuccess on success, kFailure on failure
 */
int Clogger::CreateHourLog(const std::string& filename, const std::string& dataPath, const std::string& dirName) {
    try {
        // Construct the log directory path
        std::string logDir = dataPath + "/LOG";

        // Create the directories if they don't exist
        if (mkdir(logDir.c_str(), 0777) == -1 && errno != EEXIST) {
            std::cerr << "ERROR: create directory \n" << logDir << std::endl;
            return kFailure;
        }
        
        logDir = logDir + "/" + dirName;

        // Create the directories if they don't exist
        if (mkdir(logDir.c_str(), 0777) == -1 && errno != EEXIST) {
            std::cerr << "ERROR: create directory \n" << logDir << std::endl;
            return kFailure;
        }

        std::ostringstream oss;
        oss << logDir << "/"
            << std::setfill('0') << std::setw(4) << m_nYear << "-"
            << std::setw(2) << m_nMonth << "-"
            << std::setw(2) << m_nDay;

        std::string logPath = oss.str();
        
        //std::string logPath = logDir + "/" + std::to_string(m_nYear) + "-" +
            //std::to_string(m_nMonth) + "-" + std::to_string(m_nDay);

        // Create the directories if they don't exist
        if (mkdir(logPath.c_str(), 0777) == -1 && errno != EEXIST) {
            std::cerr << "ERROR: create directory \n" << logPath << std::endl;
            return kFailure;
        }
       
        // If a filename is provided, create the log file
        if (!filename.empty()) {
            std::string fullFilePath = logPath + "/" + filename;

            // Lock the mutex to ensure thread-safe file operations
            //pthread_mutex_lock(&m_lock);
            try
            {
                // Open the log file for appending
                this->m_fp = fopen(fullFilePath.c_str(), "a+");
                printf("\nOPEN FILE:[%s]\n", fullFilePath.c_str());
                // Check if the file opened successfully
                if (!this->m_fp) {
                    std::cerr << "\nERROR: Unable to open log file at " << fullFilePath << std::endl;
                    //pthread_mutex_unlock(&m_lock);
                    return kFailure; // The lock will be released when 'lock' goes out of scope
                }
            }
            catch (...)
            {
                printf("\nOPEN FILE FAILED:[%s]", fullFilePath.c_str());
            }
            //pthread_mutex_unlock(&m_lock);
 
        }
        return kSuccess;
    }
    catch (const std::exception& e) {
        std::cerr << "Exception in Clogger::CreateHourLog(): " << e.what() << std::endl;
        return kFailure;
    }
    catch (...) {
        std::cerr << "Unknown exception in Clogger::CreateHourLog()" << std::endl;
        return kFailure;
    }
}


/**
 * Enable or disable logging
 * @param enabled - true to enable logging, false to disable
 */
void Clogger::SetLoggingEnabled(int enabled) {
    pthread_mutex_lock(&m_lock);
    try
    {
        m_loggingEnabled = enabled;
    }
    catch (...)
    {
        std::cerr << "Unknown exception in Clogger::CreateHourLog()" << std::endl;
    }
    pthread_mutex_unlock(&m_lock);
}

/**
 * Set the data path for log files
 * @param dataPath - new data path for log files
 */
void Clogger::SetDataPath(const std::string& dataPath) {
    pthread_mutex_lock(&m_lock);
    try
    {
        m_sDataPath = dataPath;
    }
    catch (...)
    {
        std::cerr << "Unknown exception in Clogger::SetDataPath()" << std::endl;
    }
    pthread_mutex_unlock(&m_lock);
}

/**
 * Set the debug level
 * @param debugLevel - new debug level
 */
void Clogger::SetDebugLevel(int debugLevel) {
    pthread_mutex_lock(&m_lock);
    try
    {
        m_debugLevel = debugLevel;
        this->m_debugLevel = debugLevel; // Update the debug level in LogWriter
    }
    catch (...)
    {
        std::cerr << "Unknown exception in Clogger::SetDebugLevel()" << std::endl;
    }
    pthread_mutex_unlock(&m_lock);
}

/**
 * Create a log file for each Channel each hour 
 */
void Clogger::CreateLog(const std::string& channelId) {
    try {
        if (m_loggingEnabled == 0) {
            return; // Skip log creation if logging is disabled
        }

        // Get the current time and convert it to local time
        auto now = std::chrono::system_clock::now();
        std::time_t now_c = std::chrono::system_clock::to_time_t(now);
        std::tm local_tm = *std::localtime(&now_c);

        // Update the date and hour variables
        m_nYear = local_tm.tm_year + 1900;
        m_nMonth = local_tm.tm_mon + 1;
        m_nDay = local_tm.tm_mday;
        m_nHour = local_tm.tm_hour;

        // Format hour for file name
        std::ostringstream hourStream;
        hourStream << std::setfill('0') << std::setw(2) << local_tm.tm_hour;

        // Construct log directory for the channel
        std::ostringstream channelLogDirStream;
        channelLogDirStream << m_sDataPath << "/LOG/" << m_sFolderName << "/"
                            << std::setfill('0') << std::setw(4) << m_nYear << "-"
                            << std::setw(2) << m_nMonth << "-"
                            << std::setw(2) << m_nDay << "/" << channelId;

        std::string channelLogDir = channelLogDirStream.str();

        // Create the channel directory if it doesn't exist
        mkdir(channelLogDir.c_str(), 0777);

        // Construct file name for the channel log
        std::string channelLogFileName = hourStream.str() + "_" + m_sFolderName + ".log";
        std::string channelLogFullPath = channelLogDir + "/" + channelLogFileName;

        // Open the channel log file for appending
        this->m_fp = fopen(channelLogFullPath.c_str(), "a+");
        if (!this->m_fp) {
            std::cerr << "ERROR: Unable to open channel log file at " << channelLogFullPath << std::endl;
            return;
        }

        // // Log entry for the channel log
        // snprintf(m_logString, sizeof(m_logString), "[DCHL][RUN] Opened Channel Log for %s:", channelId.c_str());
        // this->WriteLog(40, m_logString); // Assuming 40 is the kDebug level

    } catch (const std::exception& e) {
        std::cerr << "Exception in Clogger::CreateLog(): " << e.what() << std::endl;
    } catch (...) {
        std::cerr << "Unknown exception in Clogger::CreateLog()" << std::endl;
    }
}

