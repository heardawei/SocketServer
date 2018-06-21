#define _CRT_SECURE_NO_WARNINGS

#include "cache_part.h"
#include <stdlib.h>
#include <string.h>
#include <time.h>

#ifdef _WIN32
#include <io.h> /* remove */
#include <windows.h>
#elif defined __linux__
#include <unistd.h>
#include <sys/time.h>
#endif

struct private_cache
{
	uint64_t    capacity;
	FILE        *wfp;
	char        dir[MAX_PATH];
};

#ifdef _WIN32
static int gettimeofday(struct timeval *tp, void *tzp)
{
	time_t clock;
	struct tm tm;
	SYSTEMTIME wtm;
	GetLocalTime(&wtm);
	
	tm.tm_year  = wtm.wYear - 1900;
	tm.tm_mon   = wtm.wMonth - 1;
	tm.tm_mday  = wtm.wDay;
	tm.tm_hour  = wtm.wHour;
	tm.tm_min   = wtm.wMinute;
	tm.tm_sec   = wtm.wSecond;
	tm.tm_isdst = -1;
	clock = mktime(&tm);
	
	tp->tv_sec = (long)clock;
	tp->tv_usec = wtm.wMilliseconds * 1000;
	
	return 0;
}
#endif

static int create_cache_buffer(cache_t *p_cache)
{
	p_cache->buf = (char *)malloc((size_t)p_cache->p_priv->capacity);
	if (p_cache->buf == NULL)
	{
		return -1;
	}
	return 0;
}

static int create_cache_file(cache_t *p_cache)
{
	int result = -1;
	struct timeval now;
	gettimeofday(&now, NULL);

#ifdef _WIN32
	snprintf(p_cache->path, MAX_PATH,
		"%s\\cache_%lu_%lu", p_cache->p_priv->dir, now.tv_sec, now.tv_usec);
#else
	snprintf(p_cache->path, MAX_PATH,
		"%s/cache_%lu_%lu", p_cache->p_priv->dir, now.tv_sec, now.tv_usec);
#endif

	p_cache->p_priv->wfp = fopen(p_cache->path, "wb");

	if (NULL == p_cache->p_priv->wfp) {
#ifdef _DEBUG
		printf("open %s error:\n", p_cache->path);
		perror("fopen error");
#endif
		return result;
	}
	char c;
	fseek(p_cache->p_priv->wfp, (long)p_cache->p_priv->capacity - 1, SEEK_SET);
	if (fwrite(&c, 1, 1, p_cache->p_priv->wfp) != 1)
	{
		result = -1;
	}
	else
	{
		result = 0;
	}
	fseek(p_cache->p_priv->wfp, 0, SEEK_SET);
	printf("open %s ok: %d\n", p_cache->path, _fileno(p_cache->p_priv->wfp));
	return result;
}

cache_t *create_cache(const char *cache_dir, uint64_t capacity, storage_to to)
{
	cache_t *p_cache = (cache_t*)calloc(1, sizeof(cache_t));

	if (p_cache)
	{
		p_cache->p_priv = (private_cache_t*)calloc(1, sizeof(private_cache_t));

		p_cache->p_priv->capacity = capacity;
		p_cache->buf = NULL;

		if (cache_dir)
		{
			size_t dir_len = strlen(cache_dir) - 1;
			
			if('/' == cache_dir[dir_len] || '\\' == cache_dir[dir_len]) {
				strncpy(p_cache->p_priv->dir, cache_dir, strlen(cache_dir));
			}
		}
		else {
			strncpy(p_cache->p_priv->dir, ".", 1);
		}
		int ret = -1;
		switch (to)
		{
		case TO_AUTO:
			if (capacity > MEMORY_CACHE_LEN)
			{
				ret = create_cache_file(p_cache);
			}
			else
			{
				ret = create_cache_buffer(p_cache);
			}
			break;
		case TO_MEM:
			ret = create_cache_buffer(p_cache);
			break;
		case TO_FILE:
		case TO_SHM:
			ret = create_cache_file(p_cache);
			break;
		}
		if (ret == -1)
		{
			leave_cache(p_cache);
			return NULL;
		}
	}
	return p_cache;
}

void leave_cache(cache_t *p_cache)
{
	if (p_cache) {
		if (p_cache->p_priv) {
			free(p_cache->p_priv);
		}
		free(p_cache);
	}
}

/* return:  finish 0    unfinished 1    fail -1 */
int cache_recv_data(cache_t *p_cache, const char *p_data, uint32_t data_len)
{
	int result = -1;
	/* create cache file only for the first time */
	if (NULL == p_cache->p_priv->wfp 
		&& p_cache->p_priv->capacity > MEMORY_CACHE_LEN)
	{
		goto cache_end;
	}
	/* cache to memory */
	if (p_cache->buf)
	{
		uint64_t wlen = p_cache->p_priv->capacity - p_cache->cache_len;
		if (wlen > data_len)
		{
			wlen = data_len;
		}
		memcpy(p_cache->buf + p_cache->cache_len, p_data, (size_t)wlen);

		p_cache->cache_len += wlen;

		if  (p_cache->cache_len >= p_cache->p_priv->capacity) 
		{
			result = 0;
			goto cache_end;
		}
		return 1;
	}
	/* cache to file */
	else if(p_cache->p_priv->wfp)
	{
		p_cache->cache_len += (uint64_t)fwrite(p_data, 1, data_len, p_cache->p_priv->wfp);

		if (p_cache->cache_len >= p_cache->p_priv->capacity) 
		{
			fclose(p_cache->p_priv->wfp);
			p_cache->p_priv->wfp = NULL;
			result = 0;
			goto cache_end;
		}
		return 1;
	}
cache_end:
	return result;
}

