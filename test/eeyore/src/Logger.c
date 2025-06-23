/*
 * File:   Logger.c
 * Author: Pooh
 *
 * Created on July 11, 2018, 10:04 AM
 */

#include "Logger.h"
#include "Alloc.h"
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <inttypes.h>
#include <math.h>
#include <time.h>
#include <pthread.h>
#include <syslog.h>


LOG_CONFIG current_log_config = {
    LOG_LEVEL_INFO,
    NULL,
    "",
    {
        {LOG_LEADER_VAR_TYPE_TIME, " ["},
        {LOG_LEADER_VAR_TYPE_LEVEL, "] ("},
        {LOG_LEADER_VAR_TYPE_THREAD, ") ["},
        {LOG_LEADER_VAR_TYPE_FUNC, "]: "},
        {LOG_LEADER_VAR_TYPE_MESSAGE, ""},
        {LOG_LEADER_VAR_TYPE_NONE, ""}
    },
    {
        LogAppenderStderr,
        LogAppenderSyslog,
        NULL
    }

};

const char* log_level_tags[] = {
    "EMERGENCY",
    "ALERT",
    "CRITICAL",
    "ERROR",
    "WARNING",
    "NOTICE",
    "INFO",
    "DEBUG",
};

static char* log_current_time_with_ms (char* wrAt){

    long  ms; // Milliseconds
    time_t s;  // Seconds
    struct timespec spec;

    clock_gettime(CLOCK_REALTIME, &spec);

    s  = spec.tv_sec;
    ms = round(spec.tv_nsec / 1.0e6); // Convert nanoseconds to milliseconds
    if (ms > 999) {
        s++;
        ms = 0;
    }

    struct tm *info;
    info = localtime( &s );

    strftime(wrAt,20,"%Y-%m-%d %H:%M:%S", info);
    wrAt += strlen(wrAt);

    int len = snprintf(wrAt, 5, ":%03ld", ms);
    return wrAt + len;
}

static char* log_write_leader_trailer(char* wrAt, const char* trailer){

    strcpy( wrAt, trailer );
    return wrAt + strlen(trailer);
}

static char* log_write_leader_time(char* wrAt, const char* trailer){

    wrAt = log_current_time_with_ms(wrAt);
    return log_write_leader_trailer(wrAt, trailer);
}

static char* log_write_leader_file(char* wrAt, const char* trailer, const char* file){

    strcpy( wrAt, file);
    wrAt += strlen(file);

    return log_write_leader_trailer(wrAt, trailer);
}

static char* log_write_leader_func(char* wrAt, const char* trailer, const char* func){

    strcpy( wrAt, func);
    wrAt += strlen(func);

    return log_write_leader_trailer(wrAt, trailer);
}

static char* log_write_leader_thread(char* wrAt, const char* trailer){

    pthread_t tid = pthread_self();
    int len = snprintf(wrAt, 10, "%x", (uint16_t)((long)tid >> 6));
    wrAt += len;

    return log_write_leader_trailer(wrAt, trailer);
}

static char* log_write_leader_level(char* wrAt, const char* trailer, LOG_LEVEL level){

    strcpy( wrAt, log_level_tags[level]);
    wrAt += strlen(wrAt);

    return log_write_leader_trailer(wrAt, trailer);
}

static char* log_write_leader_message(char* wrAt, const char* trailer, const char* msgFormat){

    strcpy( wrAt, msgFormat);
    wrAt += strlen(msgFormat);

    return log_write_leader_trailer(wrAt, trailer);
}

static char* log_write_entry(char* wrAt, LOG_LEADER_ENTRY entry, const char* file, const char* funcName, LOG_LEVEL level, const char* msgFormat ){

    switch(entry.type){
        case LOG_LEADER_VAR_TYPE_TIME:
            return log_write_leader_time( wrAt, entry.trailer);

        case LOG_LEADER_VAR_TYPE_FILE:
            return log_write_leader_file(wrAt, entry.trailer, file);

        case LOG_LEADER_VAR_TYPE_FUNC:
            return log_write_leader_func( wrAt, entry.trailer, funcName);

        case LOG_LEADER_VAR_TYPE_THREAD:
            return log_write_leader_thread( wrAt, entry.trailer);

        case LOG_LEADER_VAR_TYPE_LEVEL:
            return log_write_leader_level( wrAt, entry.trailer, level);

        case LOG_LEADER_VAR_TYPE_MESSAGE:
            return log_write_leader_message( wrAt, entry.trailer, msgFormat);

        default:
            return wrAt;
    }
}

static void log_write_invalid_format_buffer(char* formatBuffer, const char * format){
    char* wrAt = formatBuffer;
    strcpy(wrAt, LOG_INVALID_LEADER_CONFIG_MSG);
    wrAt += strlen(LOG_INVALID_LEADER_CONFIG_MSG);
    strcpy(wrAt, format);
    wrAt += strlen(format);
    *(wrAt++) = '\n';
    *wrAt = '\0';

}

static void log_write_format_buffer( char* formatBuffer, const char* file, const char* funcName, LOG_LEVEL level, const char * format){

    char* wrAt = formatBuffer;
    strcpy(wrAt, current_log_config.preamble);
    wrAt += strlen(current_log_config.preamble);

    for( int indx = 0 ; current_log_config.leaderEntry[indx].type != LOG_LEADER_VAR_TYPE_NONE ; indx++ ){
        if( indx >= LOG_LEADER_VAR_MAX){
            log_write_invalid_format_buffer(formatBuffer, format);
            break;
        }
        wrAt = log_write_entry(wrAt, current_log_config.leaderEntry[indx], file, funcName, level, format );
    }

}

#define LOG_SYSLOG_FACILITY  LOG_LOCAL3

static bool log_syslog_initialized = false;
static void log_appender_syslog_init(void){
    openlog("Beacon", LOG_NDELAY, LOG_SYSLOG_FACILITY);
    log_syslog_initialized = true;
}

void LogAppenderStdout ( const char* entry, LOG_LEVEL level){
    fprintf(stdout, "%s%s%s\n", KMAG, entry, KNRM);
}

void LogAppenderStderr ( const char* entry, LOG_LEVEL level){
    fprintf(stderr, "%s%s%s\n", KRED, entry, KNRM);
}

void LogAppenderSyslog ( const char* entry, LOG_LEVEL level){

    if(!log_syslog_initialized) log_appender_syslog_init();

    syslog(level | LOG_SYSLOG_FACILITY, "%s", entry);

}

// this routine intentionally breaks (const char*  rules by writing a terminator into the string as it parses...
// the method is static to this file and only used for cracking an allocStringCopy result that gets released
// this was done to minimized alloc and release usage to one per parse
static const char* log_get_next_token ( const char** tokenStart, const char* delimeter){

    char* tokenEnd = strstr(*tokenStart, delimeter);
    if( tokenEnd == NULL) return *tokenStart;

    const char* token = (*tokenStart);
    (*tokenEnd) = '\0';

    (*tokenStart) += strlen(token);
    (*tokenStart)++;
    return token;

}

static void log_set_error_config(void){
    current_log_config.preamble = LOG_INVALID_LEADER_CONFIG_MSG;
    current_log_config.leaderEntry[0].type = LOG_LEADER_VAR_TYPE_MESSAGE;
    current_log_config.leaderEntry[0].trailer = "";
    current_log_config.leaderEntry[1].type = LOG_LEADER_VAR_TYPE_NONE;

}

static LOG_LEADER_VAR_TYPE log_get_leader_type_by_tag( const char* tag){

    if( strcmp(tag, LOG_LEADER_VAR_TYPE_TAG_TIME ) == 0) return LOG_LEADER_VAR_TYPE_TIME;
    if( strcmp(tag, LOG_LEADER_VAR_TYPE_TAG_FILE ) == 0) return LOG_LEADER_VAR_TYPE_FILE;
    if( strcmp(tag, LOG_LEADER_VAR_TYPE_TAG_FUNC ) == 0) return LOG_LEADER_VAR_TYPE_FUNC;
    if( strcmp(tag, LOG_LEADER_VAR_TYPE_TAG_THREAD ) == 0) return LOG_LEADER_VAR_TYPE_THREAD;
    if( strcmp(tag, LOG_LEADER_VAR_TYPE_TAG_LEVEL ) == 0) return LOG_LEADER_VAR_TYPE_LEVEL;
    if( strcmp(tag, LOG_LEADER_VAR_TYPE_TAG_MESSAGE ) == 0) return LOG_LEADER_VAR_TYPE_MESSAGE;

    return LOG_LEADER_VAR_TYPE_NONE;

}

//"%(asctime): [%(levelName)] (%(thread)) [%(funcName)]: %(message)"
void LogSetConfig(LOG_LEVEL level, const char* format ){

    current_log_config.thresholdLevel = level;

    releaseStringCopy(current_log_config.configString);
    current_log_config.configString = allocStringCopy(format);

    // find preamble
    const char* startToken = current_log_config.configString;
    const char* endFormat = startToken + strlen(format);

    const char* token = log_get_next_token(&startToken, "%");

    current_log_config.preamble = token;

    bool msgEntryFound = false;
    int insIndx;
    for(  insIndx = 0; startToken < endFormat && insIndx < LOG_LEADER_VAR_MAX-1 ; insIndx++  ){
        startToken++;   //  throw away  (
        const char* tokenName = log_get_next_token(&startToken, ")");   //  token name
        startToken++;   //  throw away  trailing format

        LOG_LEADER_VAR_TYPE type = log_get_leader_type_by_tag(tokenName);
        const char* tokenTrailer = log_get_next_token(&startToken, "%");   //  token trailer

        if( type != LOG_LEADER_VAR_TYPE_NONE){
            current_log_config.leaderEntry[insIndx].type = type;
            current_log_config.leaderEntry[insIndx].trailer = tokenTrailer;
            current_log_config.leaderEntry[insIndx+1].type = LOG_LEADER_VAR_TYPE_NONE;
        }

        if( type == LOG_LEADER_VAR_TYPE_MESSAGE) msgEntryFound = true;
    }

    if( !msgEntryFound){
        log_set_error_config();
    }

}

void LogAddAppender(void (*appender)(const char*, LOG_LEVEL), bool clearAppenders){

    // find insert index
    int indx;
    for( indx=0 ; !clearAppenders && current_log_config.logAppender[indx] != NULL ; indx++ );

    current_log_config.logAppender[indx++] = appender;
    current_log_config.logAppender[indx] = NULL;


}

void _LogMessageEx(const char* file, const char* funcName, LOG_LEVEL level, const char * format, ... ){

    char formatBuffer[LOG_BUFFER_MAX_SIZE];
    log_write_format_buffer( formatBuffer, file, funcName, level, format);

    char entryBuffer[LOG_BUFFER_MAX_SIZE];

    va_list argptr;
    va_start(argptr, format);
    vsnprintf(entryBuffer, LOG_BUFFER_MAX_SIZE, formatBuffer, argptr);
    va_end(argptr);

    for( int indx = 0 ; indx < LOG_MAX_APPENDERS && current_log_config.logAppender[indx] != NULL ; indx++ ){
        current_log_config.logAppender[indx](entryBuffer, level);
    }


}

void _LogHexEx(const char* file, const char* funcName, LOG_LEVEL level, char *header, uint8_t *data, int length)
{
    char dumpBuffer[201] = ""; // LOG_HEX_MAX_LINE_CHARS * 5 + 1 + LOG_HEX_MAX_HEADER_LEN
    int headerLen = 0;
    int i = 0;
    int j = 0;

    if (header != NULL)
    {
        headerLen = strlen(header);
        if (headerLen >= LOG_HEX_MAX_HEADER_LEN)
        {
            _LogMessageEx(file, funcName, LOG_LEVEL_WARNING, "Header length is too long, skipping.");
            header = NULL;
            headerLen = 0;
        }
    }

    while (i < length)
    {
        int index = 0;
        if (header != NULL)
        {
            sprintf(dumpBuffer, "%s", header);
        }
        while(j < LOG_HEX_MAX_LINE_CHARS && i < length)
        {
            sprintf(dumpBuffer + index + headerLen, "[%02X] ", data[i]);
            j++;
            i++;
            index += 5; // each HEX value takes up 5 characters
        }
        _LogMessageEx(file, funcName, level, dumpBuffer);
        dumpBuffer[0] = '\0';
        j = 0;
    }
}
