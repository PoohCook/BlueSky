/*
 * File:   Alloc.c
 * Author: Pooh
 *
 * Created on July 17, 2018, 4:23 PM
 */



#include "Alloc.h"
#include "Logger.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

const char* allocStringCopy( const char* strIn ){
    return allocStringCopyPlus(strIn, 0);
}

void releaseStringCopy( const char* str ){
    if( str != NULL )free((void*)str);
}

const char* allocStringCopyPlus( const char* strIn, int plusSize){

     if( strIn == NULL ){
        LogMessage(LOG_LEVEL_ERROR, "copy from string not specified");
        return NULL;
     }

     const char* strOut = malloc( strlen(strIn)+1+plusSize);
     if( strOut == NULL) return NULL;

     strcpy( (char*)strOut, strIn);
     return strOut;

}
