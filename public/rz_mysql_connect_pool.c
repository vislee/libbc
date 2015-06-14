/*
 *file name : rz_mysql_connect_pool.c
 *des : 
 *date : 2015-06-14
 *author : liwq (286570361)
 *notes :
 *       2013-11-28   liwq      create files
 */

/**********************************************************/
#include "rz_mysql_connect_pool.h"
#include <stdio.h>
#include <assert.h>
#include <string.h>


static rz_mysql_pool_t *
rz_mysql_pool_init(int size)
{
    rz_mysql_pool_t *p = malloc(sizeof(rz_mysql_pool_t));
    assert(p);

    p->used = 0;
    p->size = size;
    p->val  = malloc(sizeof(MYSQL*) * size);
    assert(p->val);

    p->seq = malloc(sizeof(int) * size);
    assert(p->seq);

    int i = 0;
    for (; i<size; ++i) {
        p->val[i] = NULL;
        p->seq[i] = 0;
    }

    pthread_mutex_init(&p->lock, NULL);

    return p;
}



static int 
rz_mysql_connect_add(rz_mysql_pool_t *p, MYSQL *db)
{
    pthread_mutex_lock(&p->lock);
    if (p->size == p->used) {
        pthread_mutex_unlock(&p->lock);
        return 0;
    }

    #ifdef __DEBUG__
    printf("add_pools dbhandle:%p\n", db);
    #endif

    int i = 0;
    for (; i<p->size; ++i) {
        if (p->seq[i] == 0) {
            p->seq[i] = 1;
            break;
        }
    }

    p->val[i] = db;
    ++p->used;
    pthread_mutex_unlock(&p->lock);
    return 1;
}



static MYSQL *
rz_mysql_connect_get(rz_mysql_connect_pool_s *rmcps)
{
    pthread_mutex_lock(&rmcps->lock);
    if (rmcps->used == 0) {
        pthread_mutex_unlock(&rmcps->lock);
        return NULL;
    }

    #ifdef __DEBUG__
    printf("get a Conn\n");
    #endif

    int i = 0;
    for (; i<rmcps->size; ++i) {
        if (rmcps->seq[i] == 1) {
            rmcps->seq[i] = 0;
            break;
        }
    }

    --rmcps->used;
    pthread_mutex_unlock(&rmcps->lock);
    return rmcps->val[i];
}



static int 
rz_mysql_pool_destory(rz_mysql_pool_t *p)
{
    free(p->val);
    free(p->seq);
    free(p);
    return 0;
}



int 
rz_mysql_connect_pool_init(rz_mysql_connect_pool_s *rmcps, int max_size, const char *user, 
                               const char *pswd, const char *host, int port, const char *db)
{
    assert(rmcps);

    rmcps->max_size = max_size;
    rmcps->user = strdup(user);
    rmcps->pswd = strdup(pswd);
    rmcps->host = strdup(host);
    rmcps->port = port;
    rmcps->db = strdup(db);
    //pthread_mutex_init(&rmcps->lock, NULL);
    rmcps->rmpt = rz_mysql_pool_init(rmcps->max_size);
    return 0;
}


static MYSQL *
rz_mysql_connect(rz_mysql_connect_pool_s *pl)
{
    MYSQL *db;
    db = mysql_init(NULL);

    #ifdef __DEBUG__
    printf("connect-mysql_init\n");
    #endif

    #if MYSQL_VERSION_ID >= 50013
    my_bool reconnect = 1;
    mysql_options(db, MYSQL_OPT_RECONNECT, &reconnect);
    #endif

    #ifdef __DEBUG__
    printf("host:%s  user:%s\n",pl->pHost, pl->pUser);
    #endif
    if (mysql_real_connect(db, pl->host, pl->user, pl->pswd, pl->db, pl->port, NULL, 0)) {
        return db;
    }

    perror(mysql_error(db));
    return NULL;
}



MYSQL *
rz_mysql_connect_db_get(rz_mysql_connect_pool_s *rmcps)
{
    //pthread_mutex_lock(&rmcps->lock);
    assert(rmcps);
  
    MYSQL *db;
    db = rz_mysql_connect_get(rmcps->rmpt);

    if (NULL == db) {
        db = rz_mysql_connect(rmcps);
    }

    if (0 != mysql_ping(db)) {
        perror(mysql_error(db));
        mysql_close(db);
        perror("mysql conn err");
        //pthread_mutex_unlock(&rmcps->lock);
        return NULL;
    }
    //pthread_mutex_unlock(&rmcps->lock);
    return db;
}




int rz_mysql_connect_db_recycle(rz_mysql_connect_pool_s *rmcps, MYSQL *db)
{
    //pthread_mutex_lock(&pl->lock);
    if (0 == rz_mysql_connect_add(pl->rmpt, db)) {
        mysql_close(db);
        //pthread_mutex_unlock(&pl->lock);
        return 0;
    }
    //pthread_mutex_unlock(&pl->lock);
    return 1;
}



int rz_mysql_connect_pool_destroy(rz_mysql_connect_pool_s *rmcps)
{
    MYSQL *db = NULL;
    while(rmcps->maxSize--) {
        db = get_Conn(rmcps->rmpt);
        if (!db) continue;
        mysql_close(db);
    }
    destory_pools(rmcps->rmpt);
    rmcps->maxSize = 0;

    if (rmcps->pUser) free(rmcps->pUser);
    if (rmcps->pPswd) free(rmcps->pPswd);
    if (rmcps->pHost) free(rmcps->pHost);
    if (rmcps->db)   free(rmcps->db);

    return 0;
}
