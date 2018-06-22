
#ifndef CACHE_PART_HHH
#define CACHE_PART_HHH

#include <stdio.h>
#include <stdint.h>

#define MEMORY_CACHE_LEN 1048576   /* 10mb */ 

#ifndef MAX_PATH
#define MAX_PATH 1024
#endif

enum storage_to {
	TO_AUTO = 0,
	TO_MEM,
	TO_SHM,
	TO_FILE
};

typedef struct private_cache private_cache_t;

typedef struct 
{
    size_t		cache_len;
    char        path[MAX_PATH];
	char		*buf;
    private_cache_t *p_priv;

}cache_t;

/* note:
 * "cache_buf" minimum limit is MEMORY_CACHE_LEN bytes
 * if "cache_dir" is NULL cache_file in current directory */
cache_t *create_cache(const char *cache_dir, size_t capacity, storage_to to);

/* return read Bytes */
size_t cache_pop(cache_t *p_cache, char *p_data, size_t data_len);

/* return:  finish 0    unfinished 1    fail -1 */
int cache_push(cache_t *p_cache, const char *p_data, uint32_t data_len);

/* cache buf/file need outside delete */
void leave_cache(cache_t *p_cache);

/* delete cache buf/file */
void delete_cache(cache_t *p_cache);

#endif /* CACHE_PART_HHH */
