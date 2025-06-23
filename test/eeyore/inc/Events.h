/**
 * @file   Events.h
 * @author Pooh Cook
 * @date   March 10, 2019
 * @version 0.1
 * @brief   General purpose Event utilities
 */

#ifndef Events_H
#define Events_H

#include <pthread.h>
#include <stdbool.h>

#define EVENT_NAME_LENGTH 30

/**
 * @brief  Event
 *
 * Specifies a boolean event
 *
 */
typedef struct {
    bool flag;                          /**< @brief flag that determines if the event is called or not */
    pthread_mutex_t mutex;              /**< @brief event mutex lock */
    pthread_condattr_t attr;            /**< @brief attribute for condition  */
    pthread_cond_t cond;                /**< @brief event conditional var */
    char name[EVENT_NAME_LENGTH + 1];   /**< @brief event friendly name */
} EVENT_T;


/**
 * @brief  Initialize an event
 *
 * The event is initialized on success.
 *
 * @param e            pointer to a EVENT_T type that represents the event instance
 * @param name         name that we will call the event
 *
 * @return   true if event was successfully initialized
 *
 */
bool InitEvent(EVENT_T *e, const char* name);


/**
 * @brief  Destroy an event
 *
 * @param e            pointer to a EVENT_T type that represents the event instance
 *
 * @return   none
 *
 */
void DestroyEvent(EVENT_T *e);

/**
 * @brief  Signal an event
 *
 * The event is signaled. All thatwait will be signaled
 *
 * @param e            pointer to a EVENT_T type that represents the event instance
 *
 * @return   true if event was successfully signaled, or if the event is already signaled
 *
 */
bool SignalEvent(EVENT_T *e);


/**
 * @brief  Wait for an event
 *
 * Wait for an event
 *
 * @param e            pointer to a EVENT_T type that represents the event instance
 * @param msec         number of milliseconds until expiry
 *
 * @return   true if event has occured, false if timeout
 *
 */
bool WaitForEvent(EVENT_T *e, int msec);


/**
 * @brief  Clear an event
 *
 * CLear event flags
 *
 * @param e            pointer to a EVENT_T type that represents the event instance
 *
 * @return   true if event has cleared
 *
 */
bool ClearEvent(EVENT_T *e);


bool LockEvent(EVENT_T *e);
bool UnlockEvent(EVENT_T *e);

#endif // Events_H
