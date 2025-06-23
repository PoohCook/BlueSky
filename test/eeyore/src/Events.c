/*
 * File:   Events.c
 * Author: Pooh Cook
 *
 * Created on March 10, 2019
 */


#include "Events.h"
#include "Logger.h"
#include <stddef.h>
#include <string.h>
#include <stdlib.h>

#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <limits.h>


bool InitEvent(EVENT_T *e, const char* name)
{
    if(!e || !name)
    {
        LogMessage(LOG_LEVEL_ERROR, "Invalid input");
        return false;
    }

    int len = strlen(name);
    if(len > EVENT_NAME_LENGTH)
    {
        LogMessage(LOG_LEVEL_WARNING, "Name truncated");
        len = EVENT_NAME_LENGTH;
    }

    pthread_mutexattr_t ma;
    pthread_mutexattr_init(&ma);
    pthread_mutexattr_settype(&ma, PTHREAD_MUTEX_RECURSIVE);
    pthread_mutex_init(&e->mutex, &ma);

    pthread_condattr_init(&e->attr);
#ifndef __APPLE__
    pthread_condattr_setclock(&e->attr, CLOCK_MONOTONIC);
#endif
    pthread_cond_init(&e->cond, &e->attr);

    e->flag = false;
    memcpy(e->name, name, len);
    e->name[len] = 0;
    return true;
}

void DestroyEvent(EVENT_T *e)
{
    if(!e)
    {
        LogMessage(LOG_LEVEL_ERROR, "Invalid event");
        return;
    }

    pthread_mutex_destroy(&e->mutex);
    pthread_cond_destroy(&e->cond);
    e->flag = false;

}

bool SignalEvent(EVENT_T *e)
{
    if(!e)
    {
        LogMessage(LOG_LEVEL_ERROR, "Invalid event");
        return false;
    }

    if(e->flag)
    {
        LogMessage(LOG_LEVEL_DEBUG, "Warning : Double Signal: %s", e->name);
        return true;
    }

    LogMessage(LOG_LEVEL_DEBUG, "Event Signal: %s", e->name);
    pthread_mutex_lock(&e->mutex);
    e->flag = true;
    pthread_cond_broadcast(&e->cond);
    pthread_mutex_unlock(&e->mutex);

    return true;
}

static void msecToTimespec(int msec, struct timespec *waitTime)
{
#ifndef __APPLE__
    clock_gettime(CLOCK_MONOTONIC, waitTime);
#else
    clock_gettime(CLOCK_REALTIME, waitTime);
#endif
    waitTime->tv_sec += (msec / 1000);
    waitTime->tv_nsec += (msec % 1000) * 1000000L;
    if ( waitTime->tv_nsec > 1000000000L ){
        waitTime->tv_nsec -= 1000000000L;
        waitTime->tv_sec++;
    }
}

bool WaitForEvent(EVENT_T *e, int msec)
{
    if(!e)
    {
        LogMessage(LOG_LEVEL_ERROR, "Invalid event");
        return false;
    }

    bool isTimedWait = (msec > 0);

    if(e->flag)
    {
        pthread_mutex_lock(&e->mutex);
        e->flag = false;
        pthread_mutex_unlock(&e->mutex);
        return true;
    }

    pthread_mutex_lock(&e->mutex);

    struct timespec waitTime;
    int waitReturn;

    if(isTimedWait)
    {
        msecToTimespec(msec, &waitTime);
        while(!e->flag)
        {
            waitReturn = pthread_cond_timedwait(&e->cond, &e->mutex, &waitTime);
            if(waitReturn == ETIMEDOUT)
            {
                break;
            }
            else if( waitReturn == EINVAL || waitReturn == EPERM )
            {
                LogMessage(LOG_LEVEL_ERROR,"cond timed wait failed: %d", waitReturn);
                break;
            }
        }
    }
    else
    {
        while(!e->flag)
        {
            waitReturn = pthread_cond_wait(&e->cond, &e->mutex);
            if( waitReturn == EINVAL || waitReturn == EPERM )
            {
                LogMessage(LOG_LEVEL_ERROR,"cond wait failed: %d", waitReturn);
                break;
            }
       }
    }

    bool ret = e->flag == true;

    e->flag = false;

    if(!ret)
    {
        LogMessage(LOG_LEVEL_DEBUG, "wait for %s event timed out", e->name);
    }

    pthread_mutex_unlock(&e->mutex);
    return ret;
}

bool ClearEvent(EVENT_T *e)
{
    if(!e)
    {
        LogMessage(LOG_LEVEL_ERROR, "Invalid event");
        return false;
    }

    if(e->flag)
    {
        pthread_mutex_lock(&e->mutex);
        e->flag = false;
        pthread_mutex_unlock(&e->mutex);
    }
    return true;
}

bool LockEvent(EVENT_T *e)
{
    if(!e)
    {
        LogMessage(LOG_LEVEL_ERROR, "Invalid event");
        return false;
    }

    return 0 == pthread_mutex_lock(&e->mutex);

}

bool UnlockEvent(EVENT_T *e)
{
    if(!e)
    {
        LogMessage(LOG_LEVEL_ERROR, "Invalid event");
        return false;
    }

    return 0 == pthread_mutex_unlock(&e->mutex);

}
