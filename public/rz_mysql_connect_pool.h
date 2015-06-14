/*
 *file name : rz_mysql_connect_pool.h
 *des : 
 *date : 2015-06-14
 *author : liwq (286570361)
 *notes :
 *       2013-11-28   liwq      create files
 *       2015-06-14   liwq      modify files
 */

/**********************************************************/
#ifndef __RZ_MYSQL_CONNECT_POOL_H__
#define __RZ_MYSQL_CONNECT_POOL_H__

#include <stdlib.h>
#include <mysql.h>
#include <pthread.h>

typedef struct {
    MYSQL  **val;
    int      size;
    int      used;
    int     *seq;
    pthread_mutex_t lock;
} rz_mysql_pool_t;



typedef struct {
  //pthread_mutex_t lock;
  int               max_size;
  char             *pswd;
  char             *user;
  char             *host;
  int               port;
  char             *db;
  rz_mysql_pool_t  *rmpt;
} rz_mysql_connect_pool_s;


int rz_mysql_connect_pool_init(rz_mysql_connect_pool_s *rmcps, int max_size, const char *user, 
                               const char *pswd, const char *host, int port, const char *db);

MYSQL *rz_mysql_connect_db_get(rz_mysql_connect_pool_s *rmcps);

int rz_mysql_connect_db_recycle(rz_mysql_connect_pool_s *rmcps, MYSQL *db);

int rz_mysql_connect_pool_destroy(rz_mysql_connect_pool_s *rmcps);


#endif //__CONN_POOL_H__
