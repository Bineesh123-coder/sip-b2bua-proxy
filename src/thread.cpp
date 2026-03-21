#include "thread.h"
#include <iostream>
#include <pthread.h>
#include <sched.h>
#include <cassert>

/**
 * @brief Default constructor. Initializes a Thread object with no active thread.
 */
Thread::Thread() : m_thread(0), m_started(false) {}

/**
 * @brief Virtual destructor. Ensures that the thread is properly detached if it is still running when the object is destroyed.
 */
Thread::~Thread() {
    try {
        if (m_started) {
            pthread_detach(m_thread); ///< Ensure the thread is detached if it is still running.
        }
    }
    catch (const std::exception& e) {
        std::cerr << "Exception in destructor: " << e.what() << std::endl;
    }
}

/**
 * @brief Start the thread by creating a new thread that runs the process function.
 * @return True if the thread was successfully started, false otherwise.
 */
bool Thread::start() {
    try {
        if (!m_started) {
            int result = pthread_create(&m_thread, nullptr, Thread::process, this);
            m_started = (result == 0);
            if (!m_started) {
                throw std::runtime_error("Failed to create thread");
            }
            return m_started;
        }
    }
    catch (const std::exception& e) {
        std::cerr << "Exception in start: " << e.what() << std::endl;
        return false;
    }
    return false;
}

/**
 * @brief Static function to serve as the thread entry point. Calls the run method of the Thread object.
 * @param param A pointer to the Thread object to be executed.
 * @return A void pointer, as required by pthread_create.
 */
void* Thread::process(void* param) {
    Thread* thread = static_cast<Thread*>(param);
    assert(thread != nullptr);

    try {
        thread->run(); ///< Call the run method of the Thread object.
    }
    catch (const std::exception& e) {
        std::cerr << "Exception in thread: " << e.what() << std::endl;
    }
    catch (...) {
        std::cerr << "Unknown exception in thread" << std::endl;
    }
    return nullptr;
}

/**
 * @brief Joins the thread, blocking until it finishes execution.
 */
void Thread::join() {
    try {
        if (m_started) {
            pthread_join(m_thread, nullptr); ///< Wait for the thread to finish execution.
            m_started = false;
        }
    }
    catch (const std::exception& e) {
        std::cerr << "Exception in join: " << e.what() << std::endl;
    }
}

/**
 * @brief Detaches the thread, allowing it to run independently.
 */
void Thread::detach() {
    try {
        if (m_started) {
            pthread_detach(m_thread); ///< Detach the thread to allow it to run independently.
            m_started = false;
        }
    }
    catch (const std::exception& e) {
        std::cerr << "Exception in detach: " << e.what() << std::endl;
    }
}

/**
 * @brief Sets the thread's scheduling priority.
 * @param priority The desired priority level.
 */
void Thread::setPriority(int priority) {
    try {
        if (m_started) {
            sched_param schedParam;
            schedParam.sched_priority = priority;
            pthread_setschedparam(m_thread, SCHED_OTHER, &schedParam);
        }
    }
    catch (const std::exception& e) {
        std::cerr << "Exception in setPriority: " << e.what() << std::endl;
    }
}

/**
 * @brief Sets the thread to the highest possible priority.
 */
void Thread::setHighPriority() {
    try {
        setPriority(sched_get_priority_max(SCHED_OTHER)); ///< Set the thread to the highest priority.
    }
    catch (const std::exception& e) {
        std::cerr << "Exception in setHighPriority: " << e.what() << std::endl;
    }
}

/**
 * @brief Sets the thread to an above-normal priority level.
 */
void Thread::setAboveNormalPriority() {
    try {
        int minPriority = sched_get_priority_min(SCHED_OTHER);
        int maxPriority = sched_get_priority_max(SCHED_OTHER);
        setPriority((minPriority + maxPriority) / 2); ///< Set the thread to an above-normal priority.
    }
    catch (const std::exception& e) {
        std::cerr << "Exception in setAboveNormalPriority: " << e.what() << std::endl;
    }
}

/**
 * @brief Exits the current thread.
 */
void Thread::exit() {
    try {
        if (m_started) {
            pthread_exit(nullptr); ///< Exit the current thread.
        }
    }
    catch (const std::exception& e) {
        std::cerr << "Exception in exit: " << e.what() << std::endl;
    }
}
