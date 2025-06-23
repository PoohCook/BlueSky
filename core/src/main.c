
#include "sky.h"
#include <stdio.h>

int main(void) {
    const char* sky = getSkyColor();
    if (sky != NULL) {
        printf("The sky color is: %s\n", sky);
    } else {
        printf("Failed to get the sky color.\n");
    }
    return 0;
}
