
#ifndef CACHE_PART_HHH
#define CACHE_PART_HHH

#include <stdio.h>
#include <stdint.h>

#define MEMORY_CACHE_LEN 1048576   /* 10mb */ 

#ifndef MAX_PATH
#define MAX_PATH 1024
#endif

typedef struct private_cache private_cache_t;

typedef struct 
{
    uint64_t    cache_len;
    char        path[MAX_PATH];
    
    private_cache_t *p_priv;

}cache_t;

/* note:
 * "cache_buf" minimum limit is MEMORY_CACHE_LEN bytes
 * if "cache_dir" is NULL cache_file in current directory */
cache_t *create_cache(char *cache_buf, const char *cache_dir, uint64_t packet_len);

/* return:  finish 0    unfinished 1    fail -1 */
int cache_recv_data(cache_t *p_cache, const char *p_data, uint32_t data_len);

/* cache file need outside delete */
void leave_cache(cache_t *p_cache);

#endif /* CACHE_PART_HHH */
