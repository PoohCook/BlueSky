/*
 * File:   Logger.h
 * Author: Pooh
 *
 * Created on July 11, 2018, 10:04 AM
 */

#ifndef Logger_H
#define Logger_H
#include <syslog.h>
#include <stdbool.h>
#include <stdint.h>

typedef enum{
    LOG_LEADER_VAR_TYPE_NONE = 0,
    LOG_LEADER_VAR_TYPE_TIME,
    LOG_LEADER_VAR_TYPE_FILE,
    LOG_LEADER_VAR_TYPE_FUNC,
    LOG_LEADER_VAR_TYPE_THREAD,
    LOG_LEADER_VAR_TYPE_LEVEL,
    LOG_LEADER_VAR_TYPE_MESSAGE,
}LOG_LEADER_VAR_TYPE;


#define LOG_LEADER_VAR_TYPE_TAG_TIME        "asctime"
#define LOG_LEADER_VAR_TYPE_TAG_FILE        "filename"
#define LOG_LEADER_VAR_TYPE_TAG_FUNC        "funcName"
#define LOG_LEADER_VAR_TYPE_TAG_THREAD      "thread"
#define LOG_LEADER_VAR_TYPE_TAG_LEVEL       "levelname"
#define LOG_LEADER_VAR_TYPE_TAG_MESSAGE     "message"


typedef struct{
    LOG_LEADER_VAR_TYPE type;
    const char* trailer;
}LOG_LEADER_ENTRY;

// this enum cross defines a subset of syslog error levels.
// The subset limits the set to those that are available in Python logging
//  they are listed in ascending order
typedef enum{
    LOG_LEVEL_CRITCAL   = LOG_CRIT,
    LOG_LEVEL_ERROR     = LOG_ERR,
    LOG_LEVEL_WARNING   = LOG_WARNING,
    LOG_LEVEL_INFO      = LOG_INFO,
    LOG_LEVEL_DEBUG     = LOG_DEBUG,

} LOG_LEVEL;

#define LOG_LEADER_VAR_MAX  7
#define LOG_BUFFER_MAX_SIZE  4096
#define LOG_INVALID_LEADER_CONFIG_MSG "INVALID LEADER CONFIG: "
#define LOG_MAX_APPENDERS  10
#define LOG_HEX_MAX_LINE_CHARS 30
#define LOG_HEX_MAX_HEADER_LEN 50

#define KNRM  "\x1B[0m"
#define KRED  "\x1B[31m"
#define KGRN  "\x1B[32m"
#define KYEL  "\x1B[33m"
#define KBLU  "\x1B[34m"
#define KMAG  "\x1B[35m"
#define KCYN  "\x1B[36m"
#define KWHT  "\x1B[37m"


typedef struct{
    LOG_LEVEL thresholdLevel;
    const char* configString;
    const char* preamble;
    LOG_LEADER_ENTRY leaderEntry[LOG_LEADER_VAR_MAX];
    void (*logAppender[LOG_MAX_APPENDERS])(const char* entry, LOG_LEVEL level);
}LOG_CONFIG;

extern LOG_CONFIG current_log_config;

#define LogMessage(_level, ...)  {if(_level <= current_log_config.thresholdLevel)_LogMessageEx( __FILE__, __func__, _level,  __VA_ARGS__ );}
#define LogHex(_level, _header, _data, _length)  {if(_level <= current_log_config.thresholdLevel)_LogHexEx( __FILE__, __func__, _level, _header, _data, _length );}

void LogSetConfig(LOG_LEVEL level, const char* format );
void LogAddAppender(void (*appender)(const char*, LOG_LEVEL), bool clearAppenders);

void _LogMessageEx(const char* file, const char* func, LOG_LEVEL level, const char * format, ... );
void _LogHexEx(const char* file, const char* funcName, LOG_LEVEL level, char *header, uint8_t *data, int length);

void LogAppenderStdout ( const char* entry, LOG_LEVEL level);
void LogAppenderStderr ( const char* entry, LOG_LEVEL level);
void LogAppenderSyslog ( const char* entry, LOG_LEVEL level);


#ifndef LOG_MODULE_NAME
#define LOG_MODULE_NAME     "Unknown"
#endif

#define Log(level, ...)     LogMessage(level, LOG_MODULE_NAME ": " __VA_ARGS__)
#define LogInfo(...)        Log(LOG_LEVEL_INFO, __VA_ARGS__)
#define LogWarning(...)     Log(LOG_LEVEL_WARNING, __VA_ARGS__)
#define LogError(...)       Log(LOG_LEVEL_ERROR, __VA_ARGS__)
#define LogDebug(...)       Log(LOG_LEVEL_DEBUG, __VA_ARGS__)


#endif // Logger

