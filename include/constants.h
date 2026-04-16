#ifndef CONSTANTS_H
#define CONSTANTS_H

// Success and Failure Codes
constexpr int kSuccess = 0;
constexpr int kFailure = -1;

// Debug Levels
enum DebugLevel {
    kFatalError = 0,
    kGeneralFatalError = 0,
    kGeneralError = 10,
    kWarning = 20,
    kInformation = 30,
    kDebug = 40
};

#endif // CONSTANTS_H
