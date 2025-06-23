/**
 * @file   TestMain.c
 * @author Pooh Cook
 * @date   25 January 2019 11:00 AM
 * @version 0.1
 * @brief   Test For SB3-Common C components
 */

#include <sys/stat.h>
#include <unistd.h>
#include <errno.h>
#include <pthread.h>

#include "Eeyore.h"


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#include "SpinupTests.h"
#include "ReverseTest.h"

int main(void){

    test_initialize(LOG_LEVEL_INFO);
    sleep(1);

    test_sky();

    test_reverse();

    sleep(1);

    return test_result();

}
