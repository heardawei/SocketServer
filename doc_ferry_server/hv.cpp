#include <string.h>

#include "HV.h"
#include "debug_print.h"
#include "cp_socket.h" // htonl
#include "cache_part.h"

vc::HV::HV() :
	value(nullptr), hlen(0), head_complete(false), vlen(0), value_complete(false)
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
		if (this->value)
		{
//			delete_cache(this->value);
			leave_cache(this->value);
			this->value = NULL;
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

int vc::HV::write_value(const char *p_data, int data_len)
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

	if (this->value == nullptr)
	{
		this->value = create_cache(NULL, this->header.length, TO_FILE);
		if (this->value == nullptr)
		{
			return -1;
		}
	}

	cache_t *p_cache = (cache_t *)this->value;

	wlen = cache_push(p_cache, p_data, data_len);
	if (wlen == -1)
	{
		return -1;
	}
	else if (cache_full(p_cache))
	{
		vc::HV::value_completed();
	}
	return wlen;
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
		if (this->value)
		{
			delete_cache(this->value);
			leave_cache(this->value);
			this->value = nullptr;
		}
		this->value = create_cache(NULL, header->length, TO_MEM);
		if (this->value == nullptr)
		{
			return -1;
		}
		size_t nwrite = cache_push(this->value, value, header->length);
		if (nwrite != header->length)
		{
			delete_cache(this->value);
			leave_cache(this->value);
			this->value = nullptr;
			fprintf(stderr, "cache_push %dB, expect %dB, error.", (int)nwrite, (int)header->length);
			return -1;
		}
	}
	return 0;
}