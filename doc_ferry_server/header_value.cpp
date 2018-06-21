#include "header_value.h"
#include "debug_print.h"
#include "cp_socket.h" // htonl
#include "cache_part.h"


bool HV_is_value_complete(vc::Request *req)
{
	if (req == nullptr)
	{
		return false;
	}
	return req->value_complete;
}

bool HV_is_head_complete(vc::Request *req)
{
	if (req == nullptr)
	{
		return false;
	}
	return req->head_complete;
}

bool HV_is_complete(vc::Request *req)
{
	if (req == nullptr)
	{
		return false;
	}
	return HV_is_head_complete(req) && HV_is_value_complete(req);
}

static void HV_head_completed(vc::Request *req)
{
	if (req->head_complete == false)
	{
		req->head_complete = true;
		req->header.id = ntohl(req->header.id);
		req->header.length = ntohl(req->header.length);
		if (req->header.length < DEFAULT_MAX_MEMORY_CACHE)
		{
			req->to = vc::Request::storage_to_t::TO_MEM;
		}
		else
		{
			req->to = vc::Request::storage_to_t::TO_SHM;
		}
	}
}

static void HV_value_completed(vc::Request *req)
{
	if (req->value_complete == false)
	{
		req->value_complete = true;
	}
}

static int HV_write_head(vc::Request *req, const char *p_data, int data_len)
{
	int wlen = 0;

	if (p_data == nullptr || data_len <= 0)
	{
		return 0;
	}

	if (req->hlen < (int)sizeof(req->header))
	{
		wlen = (int)sizeof(req->header) - req->hlen;
		if (wlen > data_len)
		{
			wlen = data_len;
		}
		memcpy(&req->header + req->hlen, p_data, wlen);
		req->hlen += wlen;

		if (req->hlen == (int)sizeof(req->header))
		{
			HV_head_completed(req);
			debug_print("Request header %dB is complete\n", req->hlen);
		}
	}

	return wlen;
}

static int HV_write_value_to_shm(vc::Request *req, const char *p_data, int data_len)
{
	data_len_t wlen = 0;
#if 0
	if (p_data == nullptr || data_len <= 0)
	{
		return 0;
	}

	if (HV_is_head_complete(req) == false)
	{
		return 0;
	}

	if (req->data.get() == nullptr)
	{
		req->data = std::shared_ptr<char>((char *)create_cache(NULL, NULL, req->header.length));
		if (req->data.get() == nullptr)
		{
			return -1;
		}
	}

	cache_t *p_cache = (cache_t *)req->data.get();
	if (p_cache == nullptr)
	{
		return -1;
	}

	wlen = req->header.length - (data_len_t)p_cache->cache_len;
	if (wlen < (data_len_t)data_len)
	{
		wlen = (data_len_t)data_len;
	}

	int ret = cache_recv_data(p_cache, p_data, data_len);
	if (ret == 0)
	{
		HV_value_completed(req);
	}
	else if (ret == -1)
	{
		return -1;
	}
	return wlen;
#else
	return -1;
#endif
}

static int HV_write_value_to_mem(vc::Request *req, const char *p_data, int data_len)
{
	data_len_t wlen = 0;

	if (p_data == nullptr || data_len <= 0)
	{
		return 0;
	}

	if (HV_is_head_complete(req) == false)
	{
		return 0;
	}

	if (req->data.get() == nullptr)
	{
		req->data = std::shared_ptr<char>(new char[req->header.length]);
		if (req->data.get() == nullptr)
		{
			return -1;
		}
	}

	if (req->data_size < req->header.length)
	{
		wlen = req->header.length - req->data_size;
		if (wlen > (data_len_t)data_len)
		{
			wlen = (data_len_t)data_len;
		}
		memcpy(req->data.get() + req->data_size, p_data, wlen);
		req->data_size += wlen;

		if (req->data_size == req->header.length)
		{
			HV_value_completed(req);
			debug_print("Request value %dB is complete\n", (int)req->data_size);
		}
		debug_print("%d/%dB\n", req->data_size, req->header.length);
	}

	return (int)wlen;
}

static int HV_write_value(vc::Request *req, const char *p_data, int data_len)
{
	if (p_data == nullptr || data_len <= 0)
	{
		return 0;
	}
	switch (req->to)
	{
	case vc::Request::storage_to_t::TO_FILE:
	case vc::Request::storage_to_t::TO_SHM:
		return HV_write_value_to_shm(req, p_data, data_len);
	case vc::Request::storage_to_t::TO_MEM:
		return HV_write_value_to_mem(req, p_data, data_len);
	}
	return -1;
}

int HV_write(vc::Request *req, const char *p_data, int data_len)
{
	debug_print("%s %dB\n", __func__, data_len);
	if (req == nullptr)
	{
		return 0;
	}
	int offset = 0;
	int ret = 0;

	ret = HV_write_head(req, p_data, data_len);
	if (ret == -1)
	{
		return -1;
	}

	offset += ret;

	ret = HV_write_value(req, p_data + offset, data_len - offset);
	if (ret == -1)
	{
		return -1;
	}

	offset += ret;

	return offset;
}

int HV_set(vc::Response *resp, iface::Header *header, char *value)
{
	if (resp == nullptr)
	{
		return -1;
	}
	if (header)
	{
		resp->header.id = htonl(header->id);
		resp->header.length = htonl(header->length);
	}
	if (value)
	{
		resp->errcode = *(int *)value;
	}
	return 0;
}
