#include "header_value.h"
#include "debug_print.h"
#include "cp_socket.h" // htonl



bool HV_is_head_complete(vc::Request *req)
{
	if (req == nullptr)
	{
		return false;
	}
	return req->data_capacity == req->data_size;
}

bool HV_is_value_complete(vc::Request *req)
{
	if (req == nullptr)
	{
		return false;
	}
	return req->hlen == (int)sizeof(req->header);
}

bool HV_is_complete(vc::Request *req)
{
	if (req == nullptr)
	{
		return false;
	}
	return HV_is_head_complete(req) && HV_is_value_complete(req);
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
			debug_print("Request header %dB is complete\n", req->hlen);
		}
	}

	return wlen;
}

static int HV_write_value(vc::Request *req, const char *p_data, int data_len)
{
	data_len_t wlen = 0;

	if (p_data == nullptr || data_len <= 0)
	{
		return 0;
	}

	if (req->data_capacity == 0 || req->data.get() == nullptr)
	{
		req->data = std::shared_ptr<char>(new char[req->header.length]);
		req->data_capacity = req->header.length;
	}

	if (req->data_size < req->data_capacity)
	{
		wlen = req->data_capacity - req->data_size;
		if (wlen >(data_len_t)data_len)
		{
			wlen = data_len;
		}
		memcpy(req->data.get() + req->data_size, p_data, wlen);
		req->data_size += wlen;

		if (req->data_size == req->data_capacity)
		{
			debug_print("Request value %dB is complete\n", (int)req->data_size);
		}
	}

	return (int)wlen;
}

static void HV_head_complete(vc::Request *req)
{
	if (req->head_complete == false)
	{
		req->head_complete = true;
		req->header.id = ntohl(req->header.id);
		req->header.length = ntohl(req->header.length);
	}
}

static void HV_value_complete(vc::Request *req)
{

}

int HV_write_memory(vc::Request *req, const char *p_data, int data_len)
{
	if (req == nullptr)
	{
		return 0;
	}
	int offset = 0;
	offset += HV_write_head(req, p_data, data_len);

	if (req->head_complete == false && req->hlen == sizeof(req->hlen))
	{
		HV_head_complete(req);
	}

	offset += HV_write_value(req, p_data + offset, data_len - offset);

	if (req->value_complete == false && req->data_size == req->data_capacity)
	{
		HV_value_complete(req);
	}
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
