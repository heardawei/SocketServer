#include <string.h>

#include "HV.h"
#include "debug_print.h"
#include "cp_socket.h" // htonl
#include "cache_part.h"

vc::HV::HV() :
	hlen(0), head_complete(false), vlen(0), value_complete(false)
{

}

vc::HV::~HV()
{
}

void vc::HV::clear()
{
	this->hlen = 0;
	this->vlen = 0;
	this->head_complete = false;
	this->value_complete = false;
	if (this->to == TO_FILE || this->to == TO_SHM)
	{
		cache_t **pp_cache = (cache_t **)this->value.get();
		if (pp_cache && *pp_cache)
		{
			leave_cache(*pp_cache);
		}
	}
}

bool vc::HV::is_value_complete()
{
	return this->value_complete;
}

bool vc::HV::is_head_complete()
{
	return this->head_complete;
}

bool vc::HV::is_complete()
{
	return this->is_head_complete() && this->is_value_complete();
}

void vc::HV::head_completed()
{
	if (this->head_complete == false)
	{
		this->head_complete = true;
		this->header.id = ntohl(this->header.id);
		this->header.length = ntohl(this->header.length);
		if (this->header.length < DEFAULT_MAX_MEMORY_CACHE)
		{
			this->to = TO_MEM;
		}
		else
		{
			this->to = TO_FILE;
		}
	}
}

void vc::HV::value_completed()
{
	if (this->value_complete == false)
	{
		this->value_complete = true;
	}
}

int vc::HV::write_head(const char *p_data, int data_len)
{
	int wlen = 0;

	if (p_data == nullptr || data_len <= 0)
	{
		return 0;
	}

	if (this->hlen < (int)sizeof(this->header))
	{
		wlen = (int)sizeof(this->header) - this->hlen;
		if (wlen > data_len)
		{
			wlen = data_len;
		}
		memcpy(&this->header + this->hlen, p_data, wlen);
		this->hlen += wlen;

		if (this->hlen == (int)sizeof(this->header))
		{
			vc::HV::head_completed();
			debug_print("HV header %dB is complete\n", this->hlen);
		}
	}

	return wlen;
}

int vc::HV::write_value_to_shm(const char *p_data, int data_len)
{
	data_len_t wlen = 0;
#if 1
	if (p_data == nullptr || data_len <= 0)
	{
		return 0;
	}

	if (vc::HV::is_head_complete() == false)
	{
		return 0;
	}

	if (this->value.get() == nullptr)
	{
		this->value = std::shared_ptr<char>((char *)new cache_t *(nullptr));
		if (this->value.get() == nullptr)
		{
			return -1;
		}
	}

	if (*(cache_t **)this->value.get() == nullptr)
	{
		*(cache_t **)this->value.get() = create_cache(NULL, this->header.length, TO_FILE);
	}

	cache_t *p_cache = *(cache_t **)this->value.get();
	if (p_cache == nullptr)
	{
		return -1;
	}

	wlen = this->header.length - (data_len_t)p_cache->cache_len;
	if (wlen > (data_len_t)data_len)
	{
		wlen = (data_len_t)data_len;
	}

	int ret = cache_recv_data(p_cache, p_data, data_len);
	if (ret == 0)
	{
		vc::HV::value_completed();
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

int vc::HV::write_value_to_mem(const char *p_data, int data_len)
{
	data_len_t wlen = 0;

	if (p_data == nullptr || data_len <= 0)
	{
		return 0;
	}

	if (vc::HV::is_head_complete() == false)
	{
		return 0;
	}

	if (this->value.get() == nullptr)
	{
		this->value = std::shared_ptr<char>(new char[this->header.length]);
		if (this->value.get() == nullptr)
		{
			return -1;
		}
	}

	if (this->vlen < this->header.length)
	{
		wlen = this->header.length - this->vlen;
		if (wlen >(data_len_t)data_len)
		{
			wlen = (data_len_t)data_len;
		}
		memcpy(this->value.get() + this->vlen, p_data, wlen);
		this->vlen += wlen;

		if (this->vlen == this->header.length)
		{
			vc::HV::value_completed();
			debug_print("HV value %dB is complete\n", (int)this->vlen);
		}
		debug_print("%d/%dB\n", this->vlen, this->header.length);
	}

	return (int)wlen;
}

int vc::HV::write_value(const char *p_data, int data_len)
{
	if (p_data == nullptr || data_len <= 0)
	{
		return 0;
	}

	switch (this->to)
	{
	case TO_FILE:
	case TO_SHM:
		return vc::HV::write_value_to_shm(p_data, data_len);
	case TO_MEM:
		return vc::HV::write_value_to_mem(p_data, data_len);
	}
	return -1;
}

int vc::HV::write(const char *p_data, int data_len)
{
	debug_print("%s %dB\n", __func__, data_len);

	int offset = 0;
	int ret = 0;

	ret = vc::HV::write_head(p_data, data_len);
	if (ret == -1)
	{
		return -1;
	}

	offset += ret;

	ret = vc::HV::write_value(p_data + offset, data_len - offset);
	if (ret == -1)
	{
		return -1;
	}

	offset += ret;

	return offset;
}

int vc::HV::write(iface::Header *header, char *value)
{
	if (header)
	{
		this->header.id = htonl(header->id);
		this->header.length = htonl(header->length);
	}
	if (value)
	{
		if (this->value.get() == nullptr)
		{
			this->value = std::shared_ptr<char>(new char[header->length]);
			if (this->value.get() == nullptr)
			{
				return -1;
			}
			memcpy(this->value.get(), value, header->length);
		}
	}
	return 0;
}
