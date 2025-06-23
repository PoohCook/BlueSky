/**
 * @file   Threads.h
 * @author Pooh Cook
 * @date   March 16, 2019
 * @version 0.1
 * @brief   General purpose Thread Processor
 */

#ifndef Threads_H
#define Threads_H

#include <pthread.h>
#include <stdbool.h>
#include "Events.h"

#define THREAD_POOL_SIZE 50

/**
 * @brief  Thread
 *
 * Specifies a thread to process a given function and a wait for that thread to complete
 *
 */
typedef int THREAD_T;
#define INVALID_THREAD (THREAD_T)-1


/**
 * @brief  Initialize the thread pool.
 *
 * The thread pool is required to be initialized before allocating any threads.
 *
 * @param N/A
 *
 * @return true if thread pool is initialized, or is already initialized to begin with. otherwise false.
 *
 */
bool InitThreadPool(void);



/**
 * @brief  Destroys the thread pool
 *
 * @param timeout     number of milliseconds this routine will wait for EVERY thread to be destroyed. Set to 0 if you want to wait forever
 *
 * @return true if destruction succeeded. if waiting for threads has failed, false is returned.
 *
 */
bool DestroyThreadPool(int timeout);



/**
 * @brief  Initialize a Thread and begin processing
 *
 * The Thread is created and handler begins processing after completion of this routine.
 *
 * @param t         pointer to a THREAD_T type that represents the thread instance
 * @param name      name that we will call the thread for log purposes
 * @param handler   pointer to the thread processing function. handler accept a pointer from the context parameter
 *                  and returns a pointer to any return value desired.  freeing of any allocations related to the
 *                  return pointer is the responsibility of the consumer process.
 * @param context   pointer to context data for the handler object
 *
 * @return   true if thread was successfully initialized. otherwise false. if false, no Destroy call is required.
 *
 * @code

    void* handler( void* context );   // handler function definition

   .....

     THREAD_T thread;
     TYPE_T context = {whatever};   // consumer defined context struct

     bool result =  InitThread(&thread, "my thread", handler, &context );

    ......

     ANOTHER_TYPE_T* retValue;    // consumer defined context struct

     result = WaitThreadComplete(&thread, 1000, (void**)&retValue );


 * @endcode
 */
bool InitThread(THREAD_T *t, const char* name, void* (*handler)(void*), void* context );



/**
 * @brief  Wait completion of an thread object
 *
 * This routine will return after handler function (provided at InitThread) has completed  or the timeout has expired
 *
 * @param t       pointer to a THREAD_T type that represents the thread instance
 * @param msec    number of milliseconds to wait upon thread completion
 * @param result  pointer to pointer that will receive the return value from handler. if NULL, no result will be returned.
 *
 * @return   true if handler completed. false if timeout encountered
 *
 * @code

    void* handler( void* context );   // handler function definition

   .....

     THREAD_T thread;
     TYPE_T context = {whatever};   // consumer defined context struct

     bool result =  InitThread(&thread, "my thread", handler, &context );

    ......

     ANOTHER_TYPE_T* retValue;    // consumer defined context struct

     result = WaitThreadComplete(&thread, 1000, (void**)&retValue );


 * @endcode
 */
bool WaitThreadComplete(THREAD_T *t, int msec, void** result );


/**
 * @brief  Get Thread ID of Thread Object
 *
 * @param t       pointer to a THREAD_T type that represents the thread instance
 *
 * @return   the thread ID if Thread Object is legal. INVALID_THREAD_ID otherwise
 *
 */
#define INVALID_THREAD_ID ((pthread_t)(-1))
pthread_t GetThreadID(THREAD_T *t);


// test hooks
int GetAvailableThreadsInThreadPool(void);






#endif   //  Threads_H
