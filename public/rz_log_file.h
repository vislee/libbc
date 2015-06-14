/*
 *file name : rz_log_file.h
 *des : 
 *date : 2013-06-13
 *author : liwq (286570361)
 *notes :
 *       2013-10-23   liwq      create files
 *       2013-06-13   liwq      modify files
 */

/**********************************************************/

#ifndef __RZ_LOG_FILE_H__
#define __RZ_LOG_FILE_H__


#include <stdio.h>
#include <sys/types.h>
#include <unistd.h>
#include <string.h>
#include <pthread.h>
#include <stdarg.h>

#define STR_LEN   1024
#define PATH_LEN  1024
#define NAME_LEN  512
#define BUF_LEN   4096

#define RZ_LOG_DEBUG   1
#define RZ_LOG_INFO    2
#define RZ_LOG_WARN    3
#define RZ_LOG_ERROR   4


typedef struct {
  FILE  *fp_log;
  char   curr_name[NAME_LEN+1];
  char   last_date[8+1];
  char   log_path[512+1];
  char   proc_name[128+1];
  char   proc_ID[6+1];
  int    log_level;
  int    is_def_name;
} rz_log_file_t;


int init_log(rz_log_file_t *rlft, const char *log_path, const char *proc_name, \
            const char *sys_date, const char *proc_ID, int log_level);

int print_log(rz_log_file_t *rlft, const char *file, long line, \
           const char *func, int log_level, char * fmt, ...);

int close_log(rz_log_file_t *rlft);


#define LOG_DEBUG(rlft, ...) \
        print_log(rlft, __FILE__, __LINE__, __func__, RZ_LOG_DEBUG, __VA_ARGS__)

#define LOG_INFO(rlft, ...) \
        print_log(rlft, __FILE__, __LINE__, __func__, RZ_LOG_INFO, __VA_ARGS__)

#define LOG_WARN(rlft, ...) \
        print_log(rlft, __FILE__, __LINE__, __func__, RZ_LOG_WARN, __VA_ARGS__)

#define LOG_ERROR(rlft, ...) \
        print_log(rlft, __FILE__, __LINE__, __func__, RZ_LOG_ERROR, __VA_ARGS__)


#endif  /* __RZ_LOG_FILE_H__ */
