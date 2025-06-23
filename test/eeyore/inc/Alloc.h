/**
 * @file   Alloc.h
 * @author Pooh Cook
 * @date   July 17, 2018
 * @version 0.1
 * @brief   General purpose String Allocation and release utilities.
 */

#ifndef Alloc_H
#define Alloc_H

#include <stdbool.h>

/**
 * @brief  Create an allocated string copy
 *
 * Creates a memory allocation large enough for the specified string plus its terminator, copies the
 * specified string into it, and then returns a pointer to the new allocated string
 *
 * @param strIn    pointer to the string to be copied
 *
 * @return   pointer to the allocated string or NULL if failure
 *
 */
const char* allocStringCopy( const char* strIn );

/**
 * @brief  Create an allocated string copy with additional room
 *
 * Creates a memory allocation large enough for the specified string plus its terminator plus the specified extra size,
 * copies the specified string into it, and then returns a pointer to the new allocated string buffer
 *
 * @param strIn    pointer to the string to be copied
 * @param plusSize int number of extra characters to allocate
 *
 * @return   pointer to the allocated string or NULL if failure
 *
 */
const char* allocStringCopyPlus( const char* strIn, int plusSize);

/**
 * @brief  Releases a previously allocated string copy
 *
 * Frees the memory allocated to a string. String must have been allocated by a previous call to allocStringCopy
 * or allocStringCopyPlus.  If NULL is provided, the function does nothing
 *
 * @param strIn    pointer to the string to be freed
 *
 *
 */
void releaseStringCopy( const char* strIn );


#endif // Alloc_H
