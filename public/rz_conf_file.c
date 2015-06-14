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
#include "rz_conf_file.h"

static char * 
trim(char *s)
{
    char *ps = s;
    int   l  = strlen(s);
    int   tl = l;

    while (l > 0) {
        if (isspace(*(ps + l -1))) {
            *(ps + l - 1) = '\0';
            --l;
        } else {
            break;
        }
    }

    while(isspace(*ps)) {
        ++ps;
    }

    if (tl > strlen(ps)) {
        memmove(s, ps, strlen(ps) + 1);
    }

    return s;
}


static bool 
is_note(char **s)
{
    if (*s[0] == '#' || *s[0] == '\0' || strncmp(*s, "//", 2) == 0)
        return true;

    return false;
}


static ssize_t 
read_line(char **s, size_t *n, FILE *fp)
{
    char   *ts  = NULL;
    size_t  tn    = 0;

    ssize_t l = getline(s, n, fp);
    if (l == -1) {
        return RZ_CFG_ERR;
    }

    trim(*s);
    l = strlen(*s);

    while (l >= 2 && (*s)[l-1] == '\\') {
        (*s)[l-1] = 0;
        trim(*s);
        l = strlen(*s);

        if (getline(&ts, &tn, fp) == -1) {
            if (NULL != ts)  free(ts);
            return 0;
        }

        char *ns = ts;
        trim(ns);

        ssize_t nl = strlen(ns);
        ssize_t need_len = l - 1 + nl + 1;

        if (*n < need_len) {
            while (*n < need_len)
                *n *= 2;

            *s = realloc(*s, *n);
            if (*s == NULL) {
                free(ts);
                return -1;
            }
        }
        
        strcat(*s, nl);
        l = strlen(*s);
    }

    if (ts)
        free(ts);

    return l;
}



void 
rz_cfg_free(rz_cfg_collect_t *rcct)
{
    rz_cfg_collect_t  *rcct_curr = rcct;
    rz_cfg_collect_t  *next      = NULL;

    while (rcct_curr) {
        next = rcct_curr->next;

        rz_cfg_obj_t  *rcot_curr = rcct_curr->obj;
        rz_cfg_obj_t  *arg_next  = NULL;

        while (rcot_curr) {
            arg_next = rcot_curr->next;

            free(rcot_curr->name);
            free(rcot_curr->value);
            free(rcot_curr);

            rcot_curr = arg_next;
        }

        free(rcct_curr->name);
        free(rcct_curr);

        rcct_curr = next;
    }

    return;
}


static void ini_print(rz_cfg_collect_t *rcct)
{
    rz_cfg_collect_t *rcct_curr = rcct;

    while (rcct_curr) {
        if (rcct_curr->name == NULL) {
            rcct_curr = rcct_curr->next;
            continue;
        }

        printf("[%s]\n", rcct_curr->name);

        rz_cfg_obj_t *obj = rcct_curr->obj;

        while (obj) {
          if (obj->name == NULL || obj->value == NULL) {
              continue;
          }

            printf("    %-20s = %s\n", obj->name, obj->value);
            obj = obj->next;
        }

        rcct_curr = rcct_curr->next;
    }
    return;
}



static rz_cfg_collect_t *
rz_create_collect(rz_cfg_collect_t *rcct_head, char *name)
{
    rz_cfg_collect_t *p = calloc(1, sizeof(rz_cfg_collect_t));

    if (p == NULL) {
        rz_cfg_free(rcct_head);

        return NULL;
    }

    if ((p->name = strdup(name)) == NULL) {
        rz_cfg_free(rcct_head);

        return NULL;
    }

    return p;
}



static rz_cfg_collect_t *
rz_find_collect(rz_cfg_collect_t *rcct_head, char *name)
{
    rz_cfg_collect_t *rcct_curr = rcct_head;

    while (rcct_curr) {
        if (rcct_curr->name && strcmp(rcct_curr->name, name) == 0)
            return rcct_curr;

        rcct_curr = rcct_curr->next;
    }

    return NULL;
}



static rz_cfg_obj_t *
rz_create_obj(rz_cfg_collect_t *rcct_head, char *name, char *value)
{
    rz_cfg_obj_t *p = calloc(1, sizeof(rz_cfg_obj_t));

    if (p == NULL) {
        cfg_free(rcct_head);

        return NULL;
    }

    if ((p->name = strdup(name)) == NULL) {
        cfg_free(rcct_head);

        return NULL;
    }

    if ((p->value = strdup(value)) == NULL) {
        cfg_free(rcct_head);

        return NULL;
    }

    return p;
}



static rz_cfg_obj_t *
rz_find_obj(rz_cfg_collect_t *rcct_curr, char *name)
{
    rz_cfg_obj_t *obj = rcct_curr->obj;

    while (obj) {
        if (obj->name && strcmp(obj->name, name) == 0)
            return obj;

        obj = obj->next;
    }

    return NULL;
}



rz_cfg_collect_t *
rz_cfg_load(const char *cfg_file)
{
    FILE *fp = fopen(cfg_file, "r");
    if (NULL == fp) {
        return NULL;
    }

    rz_cfg_collect_t *rcct_head = NULL;
    rz_cfg_collect_t *rcct_prev = NULL;
    rz_cfg_collect_t *rcct_curr = NULL;

    rz_cfg_obj_t *rcot_curr     = NULL;
    rz_cfg_obj_t *rcot_prev     = NULL;

    char     *line              = NULL;
    size_t    n                 = 0;
    ssize_t   len               = 0;

    while ((len = read_line(&line, &n, fp)) != -1) {
        char *s = line;

        if (is_note(&s)) {
            continue;
        }

        len = strlen(s);
        if (len>2 && s[0]=='[' && s[len-1]==']') {
            char *name = s + 1;
            *(name+len-1-1) = '\0';
            trim(name);

            if ((rcct_curr = rz_find_collect(rcct_head, name)) == NULL) {
                if ((rcct_curr = create_section(rcct_head, name)) == NULL) {
                    free(line);
                    return NULL;
                }

                if (rcct_head == NULL) {
                    rcct_head = rcct_curr;
                }

                if (rcct_prev != NULL) {
                    rcct_prev->next = rcct_curr;
                }

                rcct_prev = rcct_curr;
                rcot_prev = NULL;
            } else {
                rcot_prev = rcct_curr->obj;

                while (rcot_prev->next != NULL) {
                    rcot_prev = rcot_prev->next;
                }
            }

            continue;
        }

        char *delimiter = strchr(s, '=');
        if (delimiter == NULL) {
            continue;
        }
        *delimiter = '\0';

        char *name = s;
        trim(name);

        char *value = delimiter + 1;
        delimiter = strchr(value, '#');
        if (NULL != delimiter) {
          *delimiter = '\0';
        }
        trim(value);

        if (rcct_curr == NULL) {
            if ((rcct_curr = create_section(rcct_head, "public")) == NULL) {
                free(line);

                return NULL;
            }

            if (rcct_head == NULL) {
                rcct_head = rcct_curr;
            }
            rcct_prev = rcct_curr;
            rcot_prev = NULL;
        }

        if ((rcot_curr = find_obj(rcct_curr, name)) == NULL) {
            rcot_curr = create_obj(rcct_head, name, value);
            if (rcot_curr == NULL) {
                free(line);

                return NULL;
            }

            if (rcot_prev) {
                rcot_prev->next = rcot_curr;
            }
            if (rcct_curr->obj == NULL) {
                rcct_curr->obj = rcot_curr;
            }

            rcot_prev = rcot_curr;
        } else {
            char *old_value = rcot_curr->value;

            if ((rcot_curr->value = strdup(value)) == NULL) {
                cfg_free(rcct_head);

                free(line);

                return NULL;
            }

            free(old_value);
        }
    }

    free(line);
    fclose(fp);

    if (rcct_head == NULL) {
        if ((rcct_head = calloc(1, sizeof(rz_cfg_collect_t))) == NULL) {
            return NULL;
        }
    }

    return rcct_head;
}


int 
cfg_read_str(rz_cfg_collect_t *rcct,
        char *section, char *name, char **value, char *default_value)
{
    if (!rcct || !name || !value) {
        return -1;
    }

    if (section == NULL || *section == 0) {
        section = "public";
    }

    rz_cfg_collect_t *rcct_curr = rcct;

    while (rcct_curr) {
        if (rcct_curr->name && strcmp(section, rcct_curr->name) == 0) {
            break;
        }

        rcct_curr = rcct_curr->next;
    }

    if (rcct_curr) {
        rz_cfg_obj_t *obj = rcct_curr->obj;

        while (obj) {
            if (obj->name && obj->value && strcmp(obj->name, name) == 0) {
                #ifdef __DEBUG__
                  printf("name:%s val is:%s\n",obj->name,obj->value);
                #endif
                *value = strdup(obj->value);
                if (*value == NULL) {
                    return -1;
                }

                return 0;
            }

            obj = obj->next;
        }
    }

    if (default_value) {
        *value = strdup(default_value);
        if (*value == NULL) {
            return -1;
        }
    } else {
        *value = NULL;
    }

    return 1;
}



int 
cfg_read_strn(rz_cfg_collect_t *rcct,
        char *section, char *name, char *value, size_t n, char *default_value)
{
    char *s = NULL;
    int ret = cfg_read_str(rcct, section, name, &s, default_value);
    if (ret < 0)
        return ret;

    memset(value, 0, n);

    if (s) {
        strncpy(value, s, n);
        free(s);
    }

    return ret;
}


static int 
ini_read_num(rz_cfg_collect_t *rcct,
        char *section, char *name, void *value, bool is_unsigned)
{
    char *s = NULL;
    int ret = cfg_read_str(rcct, section, name, &s, NULL);
    if (ret == 0) {
        if (is_unsigned) {
            *(unsigned long long int*)value = strtoull(s, NULL, 0);
        } else {
            *(long long int *)value = strtoll(s, NULL, 0);
        }

        free(s);
    }

    return ret;
}

# define INI_READ_SIGNED(type) do { \
    long long int v; \
    int ret = ini_read_num(rcct, section, name, &v, false); \
    if (ret == 0) { \
        *value = (type)v; \
    } \
    else if (ret > 0) { \
        *value = default_value; \
    } \
    return ret; \
} while (0)

# define INI_READ_UNSIGNED(type) do { \
    unsigned long long int v; \
    int ret = ini_read_num(rcct, section, name, &v, true); \
    if (ret == 0) { \
        *value = (type)v; \
    } \
    else if (ret > 0) { \
        *value = default_value; \
    } \
    return ret; \
} while (0)


int 
cfg_read_int(rz_cfg_collect_t *rcct,
        char *section, char *name, int *value, int default_value)
{
    INI_READ_SIGNED(int);
}


int 
cfg_read_float(rz_cfg_collect_t *rcct,
        char *section, char *name, float *value, float default_value)
{
    char *s = NULL;
    int ret = cfg_read_str(rcct, section, name, &s, NULL);
    if (ret == 0) {
        *value = strtof(s, NULL);

        free(s);
    } else if (ret > 0) {
        *value = default_value;
    }

    return ret;
}


int cfg_read_double(rz_cfg_collect_t *rcct,
        char *section, char *name, double *value, double default_value)
{
    char *s = NULL;
    int ret = cfg_read_str(rcct, section, name, &s, NULL);
    if (ret == 0) {
        *value = strtod(s, NULL);

        free(s);
    } else if (ret > 0) {
        *value = default_value;
    }

    return ret;
}


int cfg_read_ipv4_addr(rz_cfg_collect_t *rcct,
        char *section, char *name, struct sockaddr_in *addr, char *default_value)
{
    char *s = NULL;
    int ret = cfg_read_str(rcct, section, name, &s, default_value);
    if (ret < 0) {
        return ret;
    }

    memset(addr, 0, sizeof(struct sockaddr_in));

    if (s) {
        char *ip = strtok(s, ": \t");
        if (ip == NULL) {
            free(s);

            return -1;
        }

        char *port = strtok(NULL, ": \t");
        if (port == NULL) {
            free(s);

            return -1;
        }

        addr->sin_family = AF_INET;
        if (inet_aton(ip, &addr->sin_addr) == 0) {
            free(s);

            return -1;
        }

        addr->sin_port = htons((uint16_t)atoi(port));

        free(s);
    }

    return ret;
}



int 
cfg_read_bool(rz_cfg_collect_t *rcct,
        char *section, char *name, bool *value, bool default_value)
{
    char *s = NULL;
    int ret = cfg_read_str(rcct, section, name, &s, NULL);
    if (ret == 0) {
        int i;
        for (i = 0; s[i]; ++i)
            s[i] = tolower(s[i]);

        if (strcmp(s, "true") == 0) {
            *value = true;
        } else if (strcmp(s, "false") == 0) {
            *value = false;
        } else {
            *value = default_value;
        }

        free(s);
    }
    else if (ret > 0) {
        *value = default_value;
    }

    return ret;
}
