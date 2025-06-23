/**
 * @file   Threads.c
 * @author Pooh Cook
 * @date   March 16, 2019
 * @version 0.1
 * @brief   General purpose Thread Processor
 */


#include "Threads.h"
#include "Semaphores.h"
#include "Logger.h"
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <stdio.h>
#include <stdint.h>
#include <fcntl.h>



/**
 * A Single Thread
 *
 * Specifies a thread to process a given function and a wait for that thread to complete
 *
 */
#define SEM_NAME_STR_LEN 20
#define THREAD_NAME_LENGTH  30
typedef struct {
    bool initialized;                   /* flag to signal if Thread is currently initialized  */
    B_SEMAPHORE_T available;            /* binary named semaphore that reflects the availability of the thread */
    B_SEMAPHORE_T completed;            /* event that determines if the thread has completed */
    B_SEMAPHORE_T started;              /* event that thread routine uses to determine if the thread has started */
    B_SEMAPHORE_T run_started;          /* event that thread requester uses to determine if the thread has started */
    pthread_t thread;                   /* the thread being used to process handler function  */
    void* context;                      /* context for the handler object to use in processing */
    void* (*handler)(void*);            /* pointer to the thread processing function */
    void* result;                       /* pointer returned by handler  */
    char name[THREAD_NAME_LENGTH + 1];  /* thread friendly name */
} THREAD_SINGLE_T;

/**
 * Thread Pool
 *
 * Thread pool that manages threads. The threads are spun up on demand, and all are destroyed whenever
 * it is not in use.
 *
 */
typedef struct {
    volatile bool initialized;                  /* flag that determines if the pool is initialized.  */
    pthread_mutex_t lock;                       /* locks the whole pool  */
    I_SEMAPHORE_T available_threads;            /* number of available threads  */
    THREAD_SINGLE_T threads[THREAD_POOL_SIZE];  /* statically allocated thread structs  */
} THREAD_POOL_T;

static THREAD_POOL_T pool = {
    false,                      /* The pool is not initialized at first. */
};

static void* event_processing_routine(void* arg){

    THREAD_SINGLE_T* singlet = (THREAD_SINGLE_T*)arg;

    while(singlet->initialized){

        /* Phase 1:
         *  The thread is up now and available!
         */
        ISemUpdate(&pool.available_threads, INCR );
        BSemPost(&singlet->available);/* init_single_thread() waits for this to ensure that this routine is running. */

        /* Phase 3:
         *  The thread waits for somebody to start it.
         */
        LogMessage(LOG_LEVEL_DEBUG, "THREAD(%d)PROC %d", singlet - pool.threads, __LINE__);
        BSemWait(&singlet->started, 0);
        LogMessage(LOG_LEVEL_DEBUG, "THREAD(%d)PROC %d", singlet - pool.threads, __LINE__);

        /* The thread is officially allocated. Acquire the semaphore to prohibit usage by threads
         */
        LogMessage(LOG_LEVEL_DEBUG, "THREAD(%d)PROC %d", singlet - pool.threads, __LINE__);
        BSemWait(&singlet->available, 0);
        LogMessage(LOG_LEVEL_DEBUG, "THREAD(%d)PROC %d", singlet - pool.threads, __LINE__);

        /* Phase 5:
         *  The thread signals the guy that starts the thread that the run has started
         */
        LogMessage(LOG_LEVEL_DEBUG, "Run started: %s", singlet->name);
        BSemPost(&singlet->run_started);
        LogMessage(LOG_LEVEL_DEBUG, "THREADPROC %d", __LINE__);

        /* Phase 7:
         *  The handler is run, and the result is returned.
         */
        if(singlet->handler != NULL){
            LogMessage(LOG_LEVEL_DEBUG, ">> handler running: (%s)", singlet->name);
            BSemReset(&singlet->completed);
            singlet->result =  singlet->handler(singlet->context);
            LogMessage(LOG_LEVEL_DEBUG, ">> handler done: (%s)", singlet->name);
            singlet->handler = NULL;
        } else {
            LogMessage(LOG_LEVEL_DEBUG, "Thread has no handler. Finishing immediately.");
            singlet->result = NULL;
        }

        /* Phase 8:
         *  The completion signal is sent. The timing of this can be interchanged with Phase 9.
         */
        BSemPost(&singlet->completed);
        LogMessage(LOG_LEVEL_DEBUG, "THREAD(%d)PROC %d Initialed: %d", singlet - pool.threads, __LINE__, singlet->initialized);

    }

    /* Phase 10:
     *  The thread has now officially exited. Decrease pool resource count by 1.
     */
    ISemUpdate(&pool.available_threads, DECR );
    LogMessage(LOG_LEVEL_DEBUG, "Exit THREAD(%d)PROC %d", singlet - pool.threads, __LINE__);

    return NULL;

}

#define STR_LEN 40
static bool init_single_thread(THREAD_SINGLE_T *singlet){

    singlet->initialized = true;

    BSemInit(&singlet->available, true);
    BSemInit(&singlet->run_started, false);
    BSemInit(&singlet->completed, false);
    BSemInit(&singlet->started, false);

    singlet->context = NULL;
    singlet->result = NULL;
    singlet->handler = NULL;
    singlet->name[0] = 0;

    int pthread_create_return = pthread_create(&(singlet->thread), NULL, event_processing_routine, singlet);
    if(pthread_create_return != 0){
        LogMessage(LOG_LEVEL_ERROR, "Failed to start processor thread for '%s'. return=%d errno=%d", singlet->name, pthread_create_return, errno);
        BSemDestroy(&singlet->available);
        BSemDestroy(&singlet->run_started);
        BSemDestroy(&singlet->completed);
        BSemDestroy(&singlet->started);
        return false;
    }

    /* Phase 0:
    *  Wait for pthread_create() to run event_processing_routine()
    *  event_processing_routine() posts the semaphore upon initialization.
    *  This routine waits and releases the semaphore to ensure that a single thread is
    *  allocated in the pool.
    */
    BSemWait(&singlet->available, 0);
    BSemPost(&singlet->available);

    return true;
}

static bool destroy_single_thread(THREAD_SINGLE_T *singlet, int timeout){



    if(!singlet->initialized){
        LogMessage(LOG_LEVEL_ERROR, "Thread is not initialized");
        return true;
    }

    LogMessage(LOG_LEVEL_DEBUG, "Destroying thread: %.*s available=%d", THREAD_NAME_LENGTH, singlet->name, BSemValue(&singlet->available));

    /* Phase 3-A (destroy scenario):
     *  To make the thread exit, signal the start event with singlet->initialized is false.
     *  This way, event_processing_routine() exits its main loop.
     */
    singlet->initialized = false;
    BSemPost(&singlet->started);

    /* Phase 9-A (destroy scenario):
     *  Wait for completion signal
     */
    bool thread_released = true;
    LogMessage(LOG_LEVEL_DEBUG, "Waiting for thread: %.*s", THREAD_NAME_LENGTH, singlet->name);
    if(!BSemWait(&singlet->completed, timeout)){
        LogMessage(LOG_LEVEL_ERROR, "Failed Waiting for completion of thread: %.*s", THREAD_NAME_LENGTH, singlet->name);
        thread_released = false;
    }

    LogMessage(LOG_LEVEL_DEBUG, "Done Waiting for thread: %.*s", THREAD_NAME_LENGTH, singlet->name);


    // only join thread if it signaled as released
    if( thread_released ) pthread_join(singlet->thread, NULL);

    LogMessage(LOG_LEVEL_DEBUG, "Joined thread: %.*s", THREAD_NAME_LENGTH, singlet->name);

    BSemDestroy(&singlet->available);
    BSemDestroy(&singlet->run_started);
    BSemDestroy(&singlet->completed);
    BSemDestroy(&singlet->started);

    LogMessage(LOG_LEVEL_DEBUG, "Destroy completed thread: %.*s", THREAD_NAME_LENGTH, singlet->name);

    return thread_released;

}

static inline bool wait_for_single_thread(THREAD_SINGLE_T *singlet, int msec, void** result){

    if(!singlet->initialized){
        LogMessage(LOG_LEVEL_ERROR, "Thread is not initialized");
        return false;
    }

    int tIndex = singlet - pool.threads;
    char name[THREAD_NAME_LENGTH + 1]; /* temporary name for printing */
    memcpy(name, singlet->name, THREAD_NAME_LENGTH + 1);

    LogMessage(LOG_LEVEL_DEBUG, "Waiting for thread \"%s\"(%d) (%d msec)", singlet->name, tIndex, msec);

    /* Phase 9:
     *  Wait for completion signal. The timing of this can be interchanged with Phase 8.
     */
    if(msec > 0)
    {
        if(!BSemWait(&singlet->completed, msec)){
            LogMessage(LOG_LEVEL_ERROR, "| Waiting for thread \"%s\"(%d) FAILED timeout=%d msec", singlet->name, tIndex, msec);
            LogMessage(LOG_LEVEL_ERROR, "| -- available=%d", BSemValue(&singlet->available));
            LogMessage(LOG_LEVEL_ERROR, "| -- completed=%d", BSemValue(&singlet->completed));
            LogMessage(LOG_LEVEL_ERROR, "| -- run_started=%d", BSemValue(&singlet->run_started));
            LogMessage(LOG_LEVEL_ERROR, "| -- started=%d", BSemValue(&singlet->started));

            return false;
        }

    } else {
        BSemWait(&singlet->completed, 0);
        BSemWait(&singlet->available, 0);
        BSemPost(&singlet->available);
    }

    if(result){
        *result = singlet->result;
    }

    LogMessage(LOG_LEVEL_DEBUG, "Waiting for thread success: %s(%d) remaining:%d", name, tIndex, ISemValue(&pool.available_threads));

    return true;
}

#define SEMPOOL_NAME "tpool"

static bool destroy_pool(bool check_if_initialized, int thread_pool_size, int timeout){


    if(check_if_initialized){
        if(!pool.initialized){
            return true;
        }
    }

    bool success = true;

    LogMessage(LOG_LEVEL_INFO, "Destroying thread pool. size: %d", thread_pool_size);
    for(int i = 0; i < thread_pool_size; i++){
        THREAD_SINGLE_T* singlet = (THREAD_SINGLE_T *)pool.threads + i;
        if(!destroy_single_thread(singlet, timeout)){
            LogMessage(LOG_LEVEL_ERROR, "Failed to destroy thread %d", i);
            LogMessage(LOG_LEVEL_ERROR, "| -- available=%d", BSemValue(&singlet->available));
            LogMessage(LOG_LEVEL_ERROR, "| -- completed=%d", BSemValue(&singlet->completed));
            LogMessage(LOG_LEVEL_ERROR, "| -- run_started=%d", BSemValue(&singlet->run_started));
            LogMessage(LOG_LEVEL_ERROR, "| -- started=%d", BSemValue(&singlet->started));
            success = false;
        }
    }

    LogMessage(LOG_LEVEL_INFO, "Destroyed thread pool");

    pool.initialized = false;

    ISemDestroy(&pool.available_threads);
    pthread_mutex_destroy(&pool.lock);

    return success;

}

static inline bool initialize_pool(void){

    if(pool.initialized){
        LogMessage(LOG_LEVEL_ERROR, "Pool is already Initialized")
        return false;
    }

    pthread_mutexattr_t ma;
    pthread_mutexattr_init(&ma);
    pthread_mutexattr_settype(&ma, PTHREAD_MUTEX_RECURSIVE);
    pthread_mutex_init(&pool.lock, &ma);

    ISemInit(&pool.available_threads, 0);

    /* Initialize every thread */
    int i = 0;
    for(i = 0; i < THREAD_POOL_SIZE; i++)
    {
        THREAD_SINGLE_T* singlet = (THREAD_SINGLE_T *)pool.threads + i;
        if(!init_single_thread(singlet)){
            LogMessage(LOG_LEVEL_ERROR, "thread %d init failed", i);
            break;
        }

        /* ensure the thread is in a runnable state */
        BSemWait(&singlet->available, 0);
        BSemPost(&singlet->available);
    }

    if(i < THREAD_POOL_SIZE){
        if(!destroy_pool(false, i, 0)){
            LogMessage(LOG_LEVEL_WARNING, "destroy of pool failed");
        }
        LogMessage(LOG_LEVEL_ERROR, "Thread pool init failed. not all threads were created");
        return false;
    }

    pool.initialized = true;
    LogMessage(LOG_LEVEL_INFO, "Thread pool Initialized. size: %d", THREAD_POOL_SIZE);

    return true;
}

static inline THREAD_T find_available_thread(void){

    pthread_mutex_lock(&pool.lock);

    for(THREAD_T i = 0; i < THREAD_POOL_SIZE; i++){
        THREAD_SINGLE_T *singlet = pool.threads + i;
        if(BSemValue(&singlet->available)){
            pthread_mutex_unlock(&pool.lock);
            return i;
        }
    }

    pthread_mutex_unlock(&pool.lock);
    return INVALID_THREAD;

}

#define THREAD_AVAILABLE_WAIT_DELAY_MSEC 500

static inline bool allocate_in_pool(THREAD_T *t, const char* name, void* (*handler)(void*), void* context){


    if(!pool.initialized){
        LogMessage(LOG_LEVEL_ERROR, "Failed to allocate. pool is not initialized");
        return false;
    }

    /* Phase 2:
     *  Attempt to get a thread resource. If the semaphore wait fails, there is no
     *  resource available, therefore, no threads can be allocated.
     */
    pthread_mutex_lock(&pool.lock);
    if(!ISemWaitValue(&pool.available_threads, GRTR, 0, THREAD_AVAILABLE_WAIT_DELAY_MSEC )){
        LogMessage(LOG_LEVEL_ERROR, "Failed to allocate. no more threads.");
        pthread_mutex_unlock(&pool.lock);
        return false;
    }
    ISemUpdate(&pool.available_threads, DECR );


    /* Find the index of an available thread */
    THREAD_T allocated = find_available_thread();
    LogMessage(LOG_LEVEL_DEBUG, "Allocated thread: %s(%d) remaining:%d", name, allocated, ISemValue(&pool.available_threads));
    if(allocated == INVALID_THREAD){
        LogMessage(LOG_LEVEL_ERROR, "Failed to allocate. Could not find available thread");
        *t = allocated;
        pthread_mutex_unlock(&pool.lock);
        return false;
    }

    THREAD_SINGLE_T *singlet = pool.threads + allocated;

    int len = strlen(name);
    if(len > THREAD_NAME_LENGTH){
        LogMessage(LOG_LEVEL_DEBUG, "Name truncated");
        len = THREAD_NAME_LENGTH;
    }

    memcpy(singlet->name, name, len);
    singlet->name[len] = 0;

    singlet->context = context;
    singlet->handler = handler;
    singlet->result = NULL;

    *t = allocated;

    /* Phase 3:
     *  Signal starting the thread
     */
    BSemPost(&singlet->started);

    /* Phase 5:
     *  Wait for thread to signal it has started to run
     */
    BSemWait(&singlet->run_started, 0);
    pthread_mutex_unlock(&pool.lock);

    return true;

}


// public functions

bool InitThreadPool(void){
    return initialize_pool();
}

bool DestroyThreadPool(int timeout){
    return destroy_pool(true, THREAD_POOL_SIZE, timeout);
}

bool InitThread(THREAD_T *t, const char* name, void* (*handler)(void*), void* context ){

    if(t == NULL || name == NULL || handler == NULL){
        LogMessage(LOG_LEVEL_ERROR, "Invalid input");
        return false;
    }

    if(!pool.initialized){
        LogMessage(LOG_LEVEL_ERROR, "Thread pool is not initialized");
        return false;
    }

    bool ret = allocate_in_pool(t, name, handler, context);

    return ret;

}

bool WaitThreadComplete(THREAD_T *t, int msec, void** result ){

    if(!t || msec<0 || *t < 0 || *t >= THREAD_POOL_SIZE){
        LogMessage(LOG_LEVEL_ERROR, "Invalid input");
        return false;
    }

    if(!pool.initialized){
        LogMessage(LOG_LEVEL_ERROR, "Thread pool is not initialized");
        return false;
    }

    return wait_for_single_thread(pool.threads + *t, msec, result);
}

pthread_t GetThreadID(THREAD_T *t){
    if(t == NULL || *t < 0 || *t >= THREAD_POOL_SIZE){
        return INVALID_THREAD_ID;
    }
    return pool.threads[*t].thread;
}

int GetAvailableThreadsInThreadPool(void){
    return ISemValue(&pool.available_threads);
}
