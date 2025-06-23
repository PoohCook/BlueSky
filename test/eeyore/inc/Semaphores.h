/**
 * @file   Semaphores.h
 * @author Pooh
 * @date   July 11, 2019
 * @version 0.1
 * @brief   General purpose Semaphore utilities
 */

#ifndef Semaphore_H
#define Semaphore_H

#include <pthread.h>
#include <stdbool.h>


/**
 * @brief  Binary Semaphore
 *
 * Specifies a Binary Semaphore for use in intra process signalling.  The underlying Semaphore value is a Boolean type
 * and can only assume values of true and false
 *
 */
typedef struct {
    pthread_mutex_t mutex;
    pthread_cond_t cond;
    pthread_condattr_t condattr;
    volatile bool v;
} B_SEMAPHORE_T;

/**
 * @brief  Integer Semaphore
 *
 * Specifies an Integer Semaphore for use in intra process signalling.  The underlying Semaphore value is of integer
 * type and can assume any value which is valid for a signed int
 *
 */
typedef struct {
    pthread_mutex_t mutex;
    pthread_cond_t cond;
    pthread_condattr_t condattr;
    volatile int v;
} I_SEMAPHORE_T;



/**
 * @brief  Initializes a Boolean Semaphore
 *
 * @param bsem_p     pointer to a B_SEMAPHORE_T type to be initialized
 * @param value      boolean that specifies the initial value of the semaphore
 *
 */
void BSemInit(B_SEMAPHORE_T *bsem_p, bool value);

/**
 * @brief  Destroy a Boolean Semaphore
 *
 * @param bsem_p     pointer to a B_SEMAPHORE_T type to be destroyed
 *
 */
void BSemDestroy(B_SEMAPHORE_T *bsem_p);

/**
 * @brief  Reset a Boolean Semaphore
 *
 * This method will set the semaphore value to false
 *
 * @param bsem_p     pointer to a B_SEMAPHORE_T type to be reset
 *
 */
void BSemReset(B_SEMAPHORE_T *bsem_p);

/**
 * @brief  Post a Boolean Semaphore
 *
 * This method will set the semaphore value to true and signal any consumers currently waiting on it
 *
 * @param bsem_p     pointer to a B_SEMAPHORE_T type to be set
 *
 */
void BSemPost(B_SEMAPHORE_T *bsem_p);

/**
 * @brief  Wait on a Boolean Semaphore
 *
 * This method will wait on the semaphore value to become true. It will not consume any thread resources during
 * the span of its waiting. If msec timeout is specified with a positive non 0 value, then the wait will be limited
 * to the specified span of time. Following a successful wait, the semaphore value will be reset to false.
 *
 * @param bsem_p     pointer to a B_SEMAPHORE_T type to be waited on
 * @param msec       number of miliseconds to wait. if set to less than 1, then the wait is un-timed
 *
 * @return      true if the semaphore value has been signaled to true. false in the event of timeout
 */
bool BSemWait(B_SEMAPHORE_T* bsem_p, int msec);

/**
 * @brief  Get the current value of a Boolean Semaphore
 *
 * This method will return the current true / false state of the specified semaphore
 *
 * @param bsem_p     pointer to a B_SEMAPHORE_T type to be read
 *
 * @return      the current true / false state of the semaphore
 */
bool BSemValue(B_SEMAPHORE_T *bsem_p);


/**
 * @brief  Initializes an Integer Semaphore
 *
 * @param isem_p     pointer to a I_SEMAPHORE_T type to be initialized
 * @param value      integer that specifies the initial value of the semaphore
 *
 */
void ISemInit(I_SEMAPHORE_T *isem_p, int value);

/**
 * @brief  Destroy an Integer Semaphore
 *
 * @param isem_p     pointer to a I_SEMAPHORE_T type to be destroyed
 *
 */
void ISemDestroy(I_SEMAPHORE_T *isem_p);

/**
 * @brief  Update the value of an Integer Semaphore
 *
 * This method will update the semaphore value in accordance with the op function specified and signal any consumers
 * currently waiting on it. The op function is an operation function pointer that will provide the update value
 * based upon the semaphore's current value and the value of the operand specified.
 *
 * possible operations include:  ADD, value;  INCR; DECR;
 *
 * @param isem_p     pointer to a I_SEMAPHORE_T type to be updated
 * @param op         a pointer to a function that takes two integers ( value and operand)  and returns an updated int
 * @param o          operand associated with the operation
 *
 * @return         true if update was successful. otherwise false
 */
bool ISemUpdate(I_SEMAPHORE_T *isem_p, int (*op)(int v, int o), int o );

/**
 * @brief  Wait on a value condition of an Integer Semaphore
 *
 * This method will wait on the semaphore value to compare true in accordance with the cmp function specified.
 * The cmp function is an comparison function pointer that will provide a boolean result of the semaphore value
 * based upon the semaphore's current value and the value of the compare operand specified. If msec timeout is
 * specified with a positive non 0 value, then the wait will be limited to the specified span of time.
 *
 * possible comparitors include:  GRTR, value;
 *
 * @param isem_p     pointer to a I_SEMAPHORE_T type to be waited upon
 * @param cmp        a pointer to a function that takes two integers (value and operand)  and returns a boolean
 * @param c          comparison operand associated with the comparitor
 * @param msec       number of miliseconds to wait. if set to less than 1, then the wait is un-timed
 *
 * @return      true if the semaphore value has met the true condition of the specified comparitor.
 *              false in the event of timeout
 */
bool ISemWaitValue(I_SEMAPHORE_T *isem_p, bool (*cmp)(int v, int c), int c, int msec );

/**
 * @brief  Get the current value of an Integer Semaphore
 *
 * This method will return the current integer value of the specified semaphore
 *
 * @param isem_p     pointer to a I_SEMAPHORE_T type to be read
 *
 * @return      the current integer state of the semaphore
 */
int ISemValue(I_SEMAPHORE_T *isem_p);

// operations

int ADD ( int v, int o);

#define INCR ADD, 1
#define DECR ADD, -1

//  comparators
bool GRTR ( int v, int c);


#endif   //Semaphore_H
