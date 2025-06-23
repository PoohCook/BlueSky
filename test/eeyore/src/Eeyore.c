/**
 * @file   Eeyore.c
 * @author Pooh Cook
 * @date   20 April 2018
 * @version 0.1
 * @brief   Oh bother, oh bother... testing is such a bother...
 *          Eyore is Pooh's test donkey since he never thinks anything is going work out..
 */

#include <sys/stat.h>
#include <unistd.h>
#include <errno.h>
#include <pthread.h>
#include <stdint.h>

#include "Eeyore.h"
#include "Threads.h"

int unit_test_count = 0;
int unit_test_fail_count = 0;
int unit_test_current_test_fail_count = 0;

// Series of hackish asserts to look sorta like a unit test
void unit_assert_equal( int actual, int expected, const char*msg, const char* file, const char* func, int line ){
    if( actual != expected ){
        fprintf(stderr, "%sERROR![%s:%s](ln:%d) %s; expected(%d) received(%d)%s\n", KMAG, file, func, line, msg, expected, actual, KNRM);
        if(unit_test_current_test_fail_count++ == 0 ){
            unit_test_fail_count++;
        }
    }
}

void unit_assert_not_equal( int actual, int expected, const char*msg, const char* file, const char* func, int line ){
    if( actual == expected ){
        fprintf(stderr, "%sERROR![%s:%s](ln:%d) %s; expected not equal (%d) received(%d)%s\n", KMAG, file, func, line, msg, expected, actual, KNRM);
        if(unit_test_current_test_fail_count++ == 0 ){
            unit_test_fail_count++;
        }
    }
}

void unit_assert_greater_than( int actual, int expected, const char*msg, const char* file, const char* func, int line ){
    if( actual <= expected ){
        fprintf(stderr, "%sERROR![%s:%s](ln:%d) %s; expected greater than (%d) received(%d)%s\n", KMAG, file, func, line, msg, expected, actual, KNRM);
        if(unit_test_current_test_fail_count++ == 0 ){
            unit_test_fail_count++;
        }
    }
}

void unit_assert_less_than( int actual, int expected, const char*msg, const char* file, const char* func, int line ){
    if( actual >= expected ){
        fprintf(stderr, "%sERROR![%s:%s](ln:%d) %s; expected less than(%d) received(%d)%s\n", KMAG, file, func, line, msg, expected, actual, KNRM);
        if(unit_test_current_test_fail_count++ == 0 ){
            unit_test_fail_count++;
        }
    }
}

void unit_assert_not_equal_u64( unsigned long long actual, unsigned long long expected, const char*msg, const char* file, const char* func, int line ){
    if( actual == expected ){
        fprintf(stderr, "%sERROR![%s:%s](ln:%d) %s; expected not equal (%llu) received(%llu)%s\n", KMAG, file, func, line, msg, expected, actual, KNRM);
        if(unit_test_current_test_fail_count++ == 0 ){
            unit_test_fail_count++;
        }
    }
}

void unit_assert_greater_than_u64( unsigned long long actual, unsigned long long expected, const char*msg, const char* file, const char* func, int line ){
    if( actual <= expected ){
        fprintf(stderr, "%sERROR![%s:%s](ln:%d) %s; expected greater than (%llu) received(%llu)%s\n", KMAG, file, func, line, msg, expected, actual, KNRM);
        if(unit_test_current_test_fail_count++ == 0 ){
            unit_test_fail_count++;
        }
    }
}

void unit_assert_less_than_u64( unsigned long long actual, unsigned long long expected, const char*msg, const char* file, const char* func, int line ){
    if( actual >= expected ){
        fprintf(stderr, "%sERROR![%s:%s](ln:%d) %s; expected less than(%llu) received(%llu)%s\n", KMAG, file, func, line, msg, expected, actual, KNRM);
        if(unit_test_current_test_fail_count++ == 0 ){
            unit_test_fail_count++;
        }
    }
}

void unit_assert_str_equal( const char* actual, const char* expected, const char*msg, const char* file, const char* func, int line ){
    if( actual == NULL){
        actual = "NULL";
    }
    if( expected == NULL){
        expected = "NULL";
    }

    if( strcmp(actual, expected) != 0 ){
        fprintf(stderr, "%sERROR![%s:%s](ln:%d) %s; expected(%s) received(%s)%s\n", KMAG, file, func, line, msg, expected, actual, KNRM);
        if(unit_test_current_test_fail_count++ == 0 ){
            unit_test_fail_count++;
        }
    }
}

void unit_assert_mem_equal(void* actual, void* expected, size_t size, const char*msg, const char* file, const char* func, int line  ){
    if( memcmp(actual, expected, size) != 0) {

        fprintf(stderr, "%sERROR![%s:%s](ln:%d) %s;", KRED, file, func, line, msg);
        for( int* start = (int*)actual; start < (int*)(actual)+size ; start++){
                fprintf(stderr, "%d,", *start );
        }
        fprintf(stderr, "%s\n", KNRM);

        if(unit_test_current_test_fail_count++ == 0 ){
            unit_test_fail_count++;
        }
    }
}

bool unit_assert_not_null( void* ptr, const char*msg, const char* file, const char* func, int line ){
    if( ptr == NULL ){
        fprintf(stderr, "%sERROR![%s:%s](ln:%d) %s%s\n", KMAG, file, func, line, msg, KNRM);
        if(unit_test_current_test_fail_count++ == 0 ){
            unit_test_fail_count++;
        }
        return false;
    }
    return true;
}

bool unit_assert_null( void* ptr, const char*msg, const char* file, const char* func, int line ){
    if( ptr != NULL ){
        fprintf(stderr, "%sERROR![%s:%s](ln:%d) %s%s\n", KMAG, file, func, line, msg, KNRM);
        if(unit_test_current_test_fail_count++ == 0 ){
            unit_test_fail_count++;
        }
        return false;
    }
    return true;
}

void test_initialize(LOG_LEVEL level){

    LogSetConfig(level, "%(asctime)s [%(levelname)s] [%(funcName)s]: %(message)s" );
    //LogAddAppender(LogAppenderSyslog, true);
    LogAddAppender(LogAppenderStderr, true);
    LogMessage( LOG_LEVEL_INFO, "Testing framework initialized. Oh, bother...");

    unit_test_fail_count = 0;

    InitThreadPool();

}


void unit_test_setup( const char* func){

    unit_test_count++;
    unit_test_current_test_fail_count = 0;

    printf("Test Begin ---------------- %s -------------------------\n", func);

}

int test_result(void){

    if( !DestroyThreadPool(5000) ){
        fprintf(stderr, "%sERROR! %s%s\n", KMAG, "Failed to destroy thread pool", KNRM);
        unit_test_fail_count++;
    }

    LogMessage( LOG_LEVEL_INFO, "Total Units tests: %d", unit_test_count);

    if(unit_test_fail_count > 0){
        LogMessage( LOG_LEVEL_ERROR,"%d Tests Failed!", unit_test_fail_count);
        return -1;
    }

    LogMessage( LOG_LEVEL_INFO, "No bother...");
    return 0;
}
