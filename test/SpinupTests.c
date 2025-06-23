// Compiler: GCC 11.2
#include "SpinupTests.h"
#include "Eeyore.h"
#include "sky.h"


void test_sky(void){

    test_setup();

    const char* sky = getSkyColor();
    assert_not_null( (void*)sky, "sky color is NULL");
    assert_str_equal( sky, "azure", "sky color is not azure");

}
