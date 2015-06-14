/*
 *file name : rz_conf_file.h
 *des : 
 *date : 2015-06-13
 *author : liwq (286570361)
 *notes :
 *       2013-10-21   liwq      create files
 *       2013-06-13   liwq      modify files
 */

/**********************************************************/

#ifndef __RZ_CONF_FILE_H__
#define __RZ_CONF_FILE_H__

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <ctype.h>
#include <arpa/inet.h>


#define RZ_CFG_ERR  -1
#define RZ_CFG_OK    0



typedef struct rz_cfg_obj_s  rz_cfg_obj_t;

struct rz_cfg_obj_s {
    char            *name;
    char            *value;
    rz_cfg_obj_t    *next;
};


typedef struct rz_cfg_collect_s  rz_cfg_collect_t;

typedef struct rz_cfg_collect_s
{
    char              *name;
    rz_cfg_obj_t      *obj;
    rz_cfg_collect_t  *next;
};



rz_cfg_collect_t *rz_cfg_load(const char *cfg_file);


int rz_cfg_read_str(rz_cfg_collect_t *cfgId,
        char *collect, char *name, char **value, char *default_value);


int rz_cfg_read_strn(rz_cfg_collect_t *cfgId,
        char *collect, char *name, char *value, size_t n, char *default_value);


int rz_cfg_read_int(rz_cfg_collect_t *cfgId,
        char *collect, char *name, int *value, int default_value);


int rz_cfg_read_float(rz_cfg_collect_t *cfgId,
        char *collect, char *name, float *value, float default_value);

int rz_cfg_read_double(rz_cfg_collect_t *cfgId,
        char *collect, char *name, double *value, double default_value);


int rz_cfg_read_ipv4_addr(rz_cfg_collect_t *cfgId,
        char *collect, char *name, struct sockaddr_in *addr, char *default_value);


int rz_cfg_read_bool(rz_cfg_collect_t *cfgId,
        char *collect, char *name, bool *value, bool default_value);

void rz_cfg_free(rz_cfg_collect_t *cfgId);

#endif /* __RZ_CONF_FILE_H__ */
