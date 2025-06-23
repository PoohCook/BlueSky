/**
 * @file   Semaphores.c
 * @author Pooh Cook
 * @date   July 11, 2019
 * @version 0.1
 * @brief   General purpose Semaphore utilities
 */


#include "Semaphores.h"
#include "Logger.h"
#include <stdlib.h>
#include <string.h>
#include <errno.h>


static void msecToTimespec(int msec, struct timespec *waitTime)
{
#ifndef __APPLE__
    clock_gettime(CLOCK_MONOTONIC, waitTime);
#else
    clock_gettime(CLOCK_REALTIME, waitTime);
#endif
    LogMessage(LOG_LEVEL_DEBUG, "SRC> %d %d", waitTime->tv_sec, waitTime->tv_nsec);
    waitTime->tv_sec += (msec / 1000);
    waitTime->tv_nsec += (msec % 1000) * 1000000L;
    if ( waitTime->tv_nsec > 1000000000L ){
        waitTime->tv_nsec -= 1000000000L;
        waitTime->tv_sec++;
    }
    LogMessage(LOG_LEVEL_DEBUG, "DST> %d %d", waitTime->tv_sec, waitTime->tv_nsec);
}

static void bsem_wait(B_SEMAPHORE_T *bsem_p) {
    pthread_mutex_lock(&bsem_p->mutex);
    int waitReturn;

    while(!bsem_p->v) {
        waitReturn = pthread_cond_wait(&bsem_p->cond, &bsem_p->mutex);
        if( waitReturn == EINVAL || waitReturn == EPERM )
        {
            LogMessage(LOG_LEVEL_ERROR,"cond wait failed: %d", waitReturn);
            // this is basically wrong but given the established interface, it's all that can be done
            break;
        }
    }
    bsem_p->v = false;
    pthread_mutex_unlock(&bsem_p->mutex);
}

static bool bsem_timedwait(B_SEMAPHORE_T *bsem_p, int msec){
    pthread_mutex_lock(&bsem_p->mutex);

    struct timespec waitTime;
    msecToTimespec(msec, &waitTime);
    int waitReturn;

    while(!bsem_p->v) {
        waitReturn = pthread_cond_timedwait(&bsem_p->cond, &bsem_p->mutex, &waitTime);
        if( waitReturn == ETIMEDOUT){
            pthread_mutex_unlock(&bsem_p->mutex);
            return false;
        }
        else if( waitReturn == EINVAL || waitReturn == EPERM )
        {
            LogMessage(LOG_LEVEL_ERROR,"cond wait failed: %d", waitReturn);
            pthread_mutex_unlock(&bsem_p->mutex);
            return false;
        }
    }

    bsem_p->v = false;
    pthread_mutex_unlock(&bsem_p->mutex);
    return true;
}

static void isem_wait_value(I_SEMAPHORE_T *isem_p, bool (*cmp)(int v, int c), int c) {
    pthread_mutex_lock(&isem_p->mutex);
    int waitReturn;
    while(!cmp(isem_p->v, c)) {
        waitReturn = pthread_cond_wait(&isem_p->cond, &isem_p->mutex);
        if( waitReturn == EINVAL || waitReturn == EPERM )
        {
            LogMessage(LOG_LEVEL_ERROR,"cond wait failed: %d", waitReturn);
            // this is basically wrong but given the established interface, it's all that can be done
            break;
        }
    }
    pthread_mutex_unlock(&isem_p->mutex);
}

static bool isem_timedwait_value(I_SEMAPHORE_T *isem_p, bool (*cmp)(int v, int c), int c, int msec){
    pthread_mutex_lock(&isem_p->mutex);

    struct timespec waitTime;
    msecToTimespec(msec, &waitTime);
    int waitReturn;

    while(!cmp(isem_p->v, c)) {
        waitReturn = pthread_cond_timedwait(&isem_p->cond, &isem_p->mutex, &waitTime);
        if( waitReturn == ETIMEDOUT){
            pthread_mutex_unlock(&isem_p->mutex);
            return false;
        }
        else if( waitReturn == EINVAL || waitReturn == EPERM )
        {
            LogMessage(LOG_LEVEL_ERROR,"cond wait failed: %d", waitReturn);
            pthread_mutex_unlock(&isem_p->mutex);
            return false;
        }
    }

    pthread_mutex_unlock(&isem_p->mutex);
    return true;
}


//  Public functions

void BSemInit(B_SEMAPHORE_T *bsem_p, bool value){
    pthread_mutex_init(&(bsem_p->mutex), NULL);

    pthread_condattr_init(&(bsem_p->condattr));
#ifndef __APPLE__
    pthread_condattr_setclock(&(bsem_p->condattr), CLOCK_MONOTONIC);
#endif
    pthread_cond_init(&(bsem_p->cond), &(bsem_p->condattr));
    bsem_p->v = value;
}

void BSemDestroy(B_SEMAPHORE_T *bsem_p){
    pthread_mutex_destroy(&(bsem_p->mutex));
    pthread_cond_destroy(&(bsem_p->cond));
}

void BSemReset(B_SEMAPHORE_T *bsem_p) {
    pthread_mutex_lock(&bsem_p->mutex);
    bsem_p->v = false;
    pthread_mutex_unlock(&bsem_p->mutex);
}

void BSemPost(B_SEMAPHORE_T *bsem_p) {
    pthread_mutex_lock(&bsem_p->mutex);
    bsem_p->v = true;
    pthread_cond_signal(&bsem_p->cond);
    pthread_mutex_unlock(&bsem_p->mutex);
}

bool BSemWait(B_SEMAPHORE_T* bsem_p, int msec){
    if( msec <= 0 ){
        bsem_wait(bsem_p);
        return true;

    } else {
        return  bsem_timedwait(bsem_p, msec);
    }
}

bool BSemValue(B_SEMAPHORE_T *bsem_p){
    bool val = false;
    pthread_mutex_lock(&bsem_p->mutex);
    val = bsem_p->v;
    pthread_mutex_unlock(&bsem_p->mutex);
    return val;
}





void ISemInit(I_SEMAPHORE_T *isem_p, int value){
   pthread_mutex_init(&(isem_p->mutex), NULL);

   pthread_condattr_init(&(isem_p->condattr));
#ifndef __APPLE__
   pthread_condattr_setclock(&(isem_p->condattr), CLOCK_MONOTONIC);
#endif
   pthread_cond_init(&(isem_p->cond), &(isem_p->condattr));
   isem_p->v = value;
}

void ISemDestroy(I_SEMAPHORE_T *isem_p){
    pthread_mutex_destroy(&(isem_p->mutex));
    pthread_cond_destroy(&(isem_p->cond));
}

bool ISemUpdate(I_SEMAPHORE_T *isem_p, int (*op)(int v, int o), int o ){
    if( op == NULL ){
        LogMessage(LOG_LEVEL_ERROR, "Invalid call, operation not supplied");
        return false;
    }

    pthread_mutex_lock(&isem_p->mutex);
    isem_p->v = op(isem_p->v, o);
    pthread_cond_signal(&isem_p->cond);
    pthread_mutex_unlock(&isem_p->mutex);

    return true;
}

bool ISemWaitValue(I_SEMAPHORE_T *isem_p, bool (*cmp)(int v, int c), int c, int msec ){
    if( cmp == NULL ){
        LogMessage(LOG_LEVEL_ERROR, "Invalid call, comparator not supplied");
        return false;
    }

    if( msec <= 0 ){
        isem_wait_value(isem_p, cmp, c);
        return true;

    } else {
        return  isem_timedwait_value(isem_p, cmp, c, msec);
    }

}


int ISemValue(I_SEMAPHORE_T *isem_p){
    int val = 0;
    pthread_mutex_lock(&isem_p->mutex);
    val = isem_p->v;
    pthread_mutex_unlock(&isem_p->mutex);
    return val;

}

// operations

int ADD ( int v, int o){
    return v + o;
}

//  comparators
bool GRTR ( int v, int c){
    return v > c;
}
