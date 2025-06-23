/**
 * @file   Eeyore.h
 * @author Pooh Cook
 * @date   20 April 2018
 * @version 0.1
 * @brief   Oh bother, oh bother... testing is such a bother...
 *          Eyore is Pooh's test donkey since he never thinks anything is going work out..
 */


#ifndef Eeyore_H
#define Eeyore_H

#include<stdio.h>
#include<stdlib.h>
#include<errno.h>
#include<string.h>
#include <stdbool.h>
#include "Logger.h"


extern int unit_test_count;
extern int unit_test_fail_count;
extern int unit_test_current_test_fail_count;

// Series of hackish asserts to look sorta like a unit test
void unit_assert_equal( int actual, int expected, const char*msg, const char* file, const char* func, int line );
void unit_assert_not_equal( int actual, int expected, const char*msg, const char* file, const char* func, int line );
void unit_assert_greater_than( int actual, int expected, const char*msg, const char* file, const char* func, int line );
void unit_assert_not_equal_u64( unsigned long long actual, unsigned long long expected, const char*msg, const char* file, const char* func, int line );
void unit_assert_greater_than_u64( unsigned long long actual, unsigned long long expected, const char*msg, const char* file, const char* func, int line );
void unit_assert_less_than_u64( unsigned long long actual, unsigned long long expected, const char*msg, const char* file, const char* func, int line );
void unit_assert_less_than( int actual, int expected, const char*msg, const char* file, const char* func, int line );
void unit_assert_str_equal( const char* actual, const char* expected, const char*msg, const char* file, const char* func, int line );
void unit_assert_mem_equal(void* actual, void* expected, size_t size, const char*msg, const char* file, const char* func, int line  );
bool unit_assert_not_null( void* ptr, const char*msg, const char* file, const char* func, int line );
bool unit_assert_null( void* ptr, const char*msg, const char* file, const char* func, int line );
void test_initialize(LOG_LEVEL level);
void unit_test_setup( const char* func);
int test_result(void);


#define assert_equal( actual, expected, msg)  {unit_assert_equal( actual, expected, msg, __FILE__, __func__, __LINE__);}

#define assert_not_equal( actual, expected, msg)  {unit_assert_not_equal( actual, expected, msg, __FILE__, __func__, __LINE__);}
#define assert_greater_than( actual, expected, msg)  {unit_assert_greater_than( actual, expected, msg, __FILE__, __func__, __LINE__);}
#define assert_less_than( actual, expected, msg)  {unit_assert_less_than( actual, expected, msg, __FILE__, __func__, __LINE__);}

#define assert_not_equal_u64( actual, expected, msg)  {unit_assert_not_equal_u64( actual, expected, msg, __FILE__, __func__, __LINE__);}
#define assert_greater_than_u64( actual, expected, msg)  {unit_assert_greater_than_u64( actual, expected, msg, __FILE__, __func__, __LINE__);}
#define assert_less_than_u64( actual, expected, msg)  {unit_assert_less_than_u64( actual, expected, msg, __FILE__, __func__, __LINE__);}

#define assert_str_equal( actual, expected, msg)  {unit_assert_str_equal( actual, expected, msg, __FILE__, __func__, __LINE__);}

#define assert_mem_equal( actual, expected, size, msg)  {unit_assert_mem_equal( actual, expected, size, msg, __FILE__, __func__, __LINE__);}

#define assert_not_null( ptr, msg)  unit_assert_not_null( ptr, msg, __FILE__, __func__, __LINE__)
#define assert_null( ptr, msg)  unit_assert_null( ptr, msg, __FILE__, __func__, __LINE__)


#define test_setup() {unit_test_setup(__func__);}

#endif   // Eeyore_H
