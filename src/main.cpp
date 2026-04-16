#include <iostream>
#include <exception>
#include "sip_server.h"
#include <signal.h>

SIPServer* mSipServer = NULL;
bool started = true;

// Function prototypes
void handle_termination_signal(int signal);
int StartSipProxyService();
void cleanup_and_exit();
void setup_signal_handlers();

/* starts the Synway Recorder service*/
int StartSipProxyService() {
    int ret = kSuccess;
    try {
        mSipServer = new SIPServer(); 
        if (!mSipServer) {
            std::cerr << "Error: Failed to allocate  StartSipProxy.\n";
            return kFailure; // Assuming kFailure is defined in synway_analog_recorder.h
        }
        ret = mSipServer->Start(); // Start the recorder service
        if (ret != kSuccess) {
            std::cerr << "Error: StartSipProxyService::Start() failed with code: " << ret << "\n";
            delete mSipServer; // Free allocated memory
            mSipServer = nullptr; // Prevent dangling pointer
            return kFailure;
        }
    }
    catch (const std::exception& e) {
        std::cerr << "StartSipProxyService::EXCEPTION: " << e.what() << "\n";
        return kFailure;
    }
    catch (...) {
        std::cerr << "StartSipProxyService::UNKNOWN EXCEPTION\n";
        return kFailure;
    }
    return ret;
}

// Cleanup function to be called before exiting
void cleanup_and_exit() {
    try {
        std::cout << "Cleaning up...\n";

        if (mSipServer) {
            mSipServer->Stop(); //Stop the Recorder service
            delete mSipServer; // Free allocated memory
            mSipServer = nullptr; // Prevent dangling pointer
        }

        std::cout << "cleanup_and_exit...Complete\n";

    }
    catch (const std::exception& e) {
        std::cerr << "cleanup_and_exit Exception: " << e.what() << ". Exiting program.\n";
    }
    catch (...) {
        std::cerr << "cleanup_and_exit UNKNOWN Exception. Exiting program.\n";
    }
}

/* Sets up signal handlers for various termination signals */
void setup_signal_handlers() {
    try {
        struct sigaction sa = {};
        sa.sa_handler = handle_termination_signal; // Set the signal handler function
        sigemptyset(&sa.sa_mask); // Clear all signals from the mask
        sa.sa_flags = 0; // No special flags

        
        // Register signal handlers for common termination and error signals
        sigaction(SIGINT, &sa, nullptr);  // Interrupt (Ctrl+C)
        sigaction(SIGTERM, &sa, nullptr); // Termination request
        sigaction(SIGQUIT, &sa, nullptr); // Quit signal (Ctrl+\)
        sigaction(SIGABRT, &sa, nullptr); // Abort signal (abort())
        sigaction(SIGSEGV, &sa, nullptr); // Segmentation fault (invalid memory access)
        sigaction(SIGILL, &sa, nullptr);  // Illegal instruction
        sigaction(SIGFPE, &sa, nullptr);  // Floating-point exception (e.g., divide by zero)
        sigaction(SIGHUP, &sa, nullptr);  // Hangup detected (terminal closed)
    }
    catch (const std::exception& e) {
        std::cerr << "setup_signal_handlers::EXCEPTION: " << e.what() << "\n";
    }
    catch (...) {
        std::cerr << "setup_signal_handlers::UNKNOWN EXCEPTION\n";
    }
}

/* handle the termination signal  */
void handle_termination_signal(int signal) {
    try {
        switch (signal) {
        case SIGINT:
            std::cout << "Caught SIGINT (Ctrl+C). Exiting program.\n";
            started = false;
            break;
        case SIGTERM:
            std::cout << "Caught SIGTERM. Exiting program.\n";
            started = false;
            break;
        case SIGQUIT:
            std::cout << "Caught SIGQUIT (Ctrl+\\). Exiting program.\n";
            started = false;
            break;
        case SIGABRT:
            std::cout << "Caught SIGABRT. Exiting program.\n";
            started = false;
            break;
        case SIGSEGV:
            std::cout << "Caught SIGSEGV. Exiting program.\n";
            started = false;
            break;
        case SIGILL:
            std::cout << "Caught SIGILL. Exiting program.\n";
            started = false;
            break;
        case SIGFPE:
            std::cout << "Caught SIGFPE. Exiting program.\n";
            started = false;
            break;
        case SIGHUP:
            std::cout << "Caught SIGHUP. Exiting program.\n";
            started = false;
            break;
        default:
            std::cout << "Caught unknown signal: " << signal << "\n";
            started = false;
            break;
        }
    }
    catch (const std::exception& e) {
        std::cerr << "handle_termination_signal Exception: " << e.what() << ". Exiting program.\n";
    }
    catch (...) {
        std::cerr << "handle_termination_signal UNKNOWN Exception. Exiting program.\n";
    }
}

/* Main function to initialize, run, and manage the Synway Recorder service */
int main()
{
    try
    {
        setup_signal_handlers(); // Setup signal handlers to handle termination and critical errors gracefully

        std::cout << "Program started. Press Ctrl+C to stop or use other signals.\n";

        
        // Start the Synway Recorder service
        int ret = StartSipProxyService();
        if (ret == kFailure) {
            cleanup_and_exit(); // Cleanup and exit if initialization fails
            return 1;
        }

        // Re-register SIGINT handler after library initialization
        struct sigaction sa = {};
        sa.sa_handler = handle_termination_signal;
        sigemptyset(&sa.sa_mask);
        sa.sa_flags = 0;
        if (sigaction(SIGINT, &sa, nullptr) == -1) {
            std::cerr << "Failed to re-register SIGINT handler\n";
        } else {
            std::cout << "SIGINT handler re-registered successfully\n";
        }

        // Main loop to monitor program execution and handle settings changes
        while (started)
        {
            //sleep(1);
            std::this_thread::sleep_for(std::chrono::milliseconds(1000));
            //pthread_mutex_lock(&g_csSettingsChange); // Lock mutex before accessing shared settings
            
        }

        cleanup_and_exit(); // Cleanup resources before exiting

    }
    catch (const std::exception& e) {
        std::cerr << "Main Exception2: " << e.what() << ". Exiting program.\n";
        cleanup_and_exit(); // Cleanup resources before exiting
    }
    catch (...) {
        std::cerr << "Main Exception2: UNKNOWN. Exiting program.\n";
        cleanup_and_exit(); // Cleanup resources before exiting
    }
    return 0;
}
