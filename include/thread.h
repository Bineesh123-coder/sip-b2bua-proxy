#ifndef THREAD_H
#define THREAD_H

#include <pthread.h>
#include <stdexcept>

/**
 * @class Thread
 * @brief A base class for creating and managing threads using POSIX threads (pthreads).
 *
 * This class provides an interface for creating and managing threads in C++ using the
 * pthread library. Derived classes must implement the run() method to define the thread's
 * behavior. The class provides methods to start, join, and set the priority of threads.
 */
class Thread
{
public:
    Thread(); ///< Default constructor.
    virtual ~Thread(); ///< Virtual destructor.

    bool start(); ///< Starts the thread by calling the process function.
    void join(); ///< Joins the thread, blocking until it finishes execution.
    void detach(); ///< Detaches the thread, allowing it to run independently.

    void setPriority(int priority); ///< Sets the thread's scheduling priority.
    void setHighPriority(); ///< Sets the thread to the highest possible priority.
    void setAboveNormalPriority(); ///< Sets the thread to an above-normal priority level.
    void exit(); ///< Exits the current thread.

protected:
    virtual void run() = 0; ///< Pure virtual function to define the thread's behavior. Must be overridden by derived classes.

private:
    static void* process(void* param); ///< Static function to serve as the thread entry point.

    pthread_t m_thread; ///< Handle to the POSIX thread.
    bool m_started; ///< Flag indicating whether the thread has been started.
    
};

#endif // THREAD_H
