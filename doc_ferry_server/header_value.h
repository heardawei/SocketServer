#pragma once

#include "interface.h"
#include <memory>

namespace vc
{
#define DEFAULT_MAX_MEMORY_CACHE	((data_len_t)(50 * 1024 * 1024))

	struct Request
	{
		enum storage_to_t {
			TO_MEM,
			TO_SHM,
			TO_FILE
		};
		storage_to_t	to;

		iface::Header	header;
		int		hlen;
		bool head_complete;

		std::shared_ptr<char> data;
		data_len_t data_capacity;
		data_len_t data_size;
		bool value_complete;
	};

	struct Response
	{
		iface::Header	header;
		int		errcode;
	};

}

bool HV_is_value_complete(vc::Request *req);
bool HV_is_head_complete(vc::Request *req);
bool HV_is_complete(vc::Request *req);
int HV_write(vc::Request *req, const char *p_data, int data_len);
int HV_set(vc::Response *resp, iface::Header *header, char *value);