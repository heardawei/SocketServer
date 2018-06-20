#pragma once

#include "interface.h"
#include <memory>

namespace vc
{

	typedef struct
	{
		iface::Header	header;
		int		hlen;
		bool head_complete;

		std::shared_ptr<char> data;
		data_len_t data_capacity;
		data_len_t data_size;
		bool value_complete;
	}Request;

	typedef struct
	{
		iface::Header	header;
		int		errcode;
	}Response;

}

bool HV_is_value_complete(vc::Request *req);
bool HV_is_head_complete(vc::Request *req);
bool HV_is_complete(vc::Request *req);
int HV_write_memory(vc::Request *req, const char *p_data, int data_len);
int HV_set(vc::Response *resp, iface::Header *header, char *value);