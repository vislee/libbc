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

#include "rz_log_file.h"


static char * 
create_log_name(char *file_name, rz_log_file_t *rlft)
{
    sprintf(file_name, "%s_%8.8s_%s.log", rlft->proc_name, \
            rlft->last_date, rlft->proc_ID);

    return file_name;
}

static void 
backup_logs(rz_log_file_t *rltf) {
    char *new_log[PATH_LEN] = {0};
    char *old_log[PATH_LEN] = {0};

    snprintf(old_log, PATH_LEN, "%s/%s", rltf->log_path, rltf->curr_name);
    snprintf(new_log, PATH_LEN, "%s/%s_%s", rltf->log_path, rltf->last_date, rltf->curr_name);

    rename(old_log, new_log);

    return;
}


static FILE 
*open_log(rz_log_file_t *rlft, const char *log_name)
{
    char logs[PATH_LEN] = {0};

    strncpy(logs, PATH_LEN-1, rlft->log_path);

    if (logs[strlen(logs)-1] != '/') {
        logs[strlen(logs)] = '/';
    }

    if (rlft->is_def_name == 1) {
        strcat(logs, rltf->curr_name);
    }else if (NULL == log_name || strlen(log_name) == 0) {
        create_log_name(rlft->curr_name, rlft);
        strcat(logs, rlft->curr_name);
    } else {
        strcat(logs, log_name);
        strncpy(rlft->curr_name, NAME_LEN, log_name);
        rlft->is_def_name = 1;
    }
    rlft->fp_log = fopen(logs, "a+");

    if (NULL == rlft->fp_log) {
        fprintf(stderr, "open logs name '%s' return null", logs);
        return NULL;
    }

    setvbuf(rlft->fp_log, NULL, _IOLBF, BUF_LEN);

    return rlft->fp_log;
}



int init_log(rz_log_file_t *rlft, const char *log_path, const char *proc_name, \
            const char *sys_date, const char *proc_ID, int log_level)
{
    memset(rlft, 0x00, sizeof(rz_log_file_t));

    strcpy(rlft->log_path, log_path);
    strcpy(rlft->proc_name, proc_name);
    strcpy(rlft->proc_ID, proc_ID);
    strncpy(rlft->last_date, sys_date, 8);
    rlft->last_date[8] = 0;
    rlft->log_level = log_level;

    // rlft->fpLog = open_log(rlft);
    // if (!rlft->fpLog) {
    //   perror("initLog open err!");
    //   return -1;
    // }

    return 0;
}


static int 
get_log_level(char *log_level_info, int log_level)
{
    switch(log_level) {
        case RZ_LOG_DEBUG:
            strcpy(log_level_info, "DEBUG");
            break;
        case RZ_LOG_INFO:
            strcpy(log_level_info, "INFO");
            break;
        case RZ_LOG_WARN:
            strcpy(log_level_info, "WARN");
            break;
        case RZ_LOG_ERROR:
            strcpy(log_level_info, "ERROR");
            break;
        default:
            perror("error log level!");
    }
    return 0;
}



static char* 
get_now_time(char *t)
{
    if (!t) return NULL;

    time_t sec;
    struct tm *t;

    sec = time(NULL);
    t = localtime(&sec);

    sprintf(t,"%04d%02d%02d%02d%02d%02d", \
              t->tm_year+1900,t->tm_mon+1,\
              t->tm_mday, t->tm_hour,t->tm_min,t->tm_sec);

    t[14]='\0';

    return t;
}


int 
print_log(rz_log_file_t *rlft, const char *file, long line, \
           const char *func, int log_level, char * fmt, ...)
{
    va_list args;
    char log_buf[BUF_LEN] = {0};
    char log_args[BUF_LEN] = {0};

    char now_time[14+1];
    char log_level_info[7];

    if (rlft->log_level > log_level) {
        return 0;
    }

    get_now_time(now_time);

    if (strncmp(now_time,rlft->lastDate,8) != 0 ) {
        fclose(rlft->fp_log);

        backup_logs(rlft);

        strncpy(rlft->last_date, now_time, 8);
        rlft->last_date[8]='\0';

        if(open_log(rlft, NULL) == NULL) {
            perror("print_log: Can't open log file!");
            return -1;
        }
    }

    memset(log_level_info, 0x00, sizeof(log_level_info));
    get_log_level(log_level_info, log_level);

    snprintf(log_buf, BUF_LEN, "[%6.6s][%8.8s %2.2s:%2.2s:%2.2s]", 
             rlft->proc_ID, now_time, now_time+8, now_time+10, now_time+12);

    if (rlft->log_level <= DEBUG) {
        snprintf(log_buf, BUF_LEN, "%s [%s:%s:%ld]", log_buf, file, func, line);
    }

    snprintf(log_buf, BUF_LEN, "%s [%6.6s]: ", log_buf, log_level_info);
  
    va_start(args, fmt);
    vsnprintf(log_args, BUF_LEN, fmt, args); 
    va_end(args);

    snprintf(log_buf, BUF_LEN, "%s %s\n", log_buf, log_args);
    fputs(log_buf, rlft->fp_log);
    // fflush(rlft->fp_log);

    return 0;
}

int clear_log(rz_log_file_t *rlft)
{
    // fflush(rlft->fp_logs);
    if (0 != fclose(rlft->fp_log)) {
        perror("close log file err!");
        return -1;
    }
    memset(rlft, 0x00, sizeof(rz_log_file_t));
    return 0;
}
